class WasmGame {
    constructor(name, width, height) {
        this.name = name;
        this.width = width;
        this.height = height;
        this.events = [];
    }

    eventVariants = {
        "Escape": 2,
        "ArrowUp": 3,
        "ArrowDown": 4,
        "ArrowLeft": 5,
        "ArrowRight": 6,
        " ": 7,
        "F1": 8,
        "F2": 9,
        "F3": 10,
        "F4": 11,
        "F5": 12,
        "F6": 13,
        "F10": 14,
    };

    palette = [
        [0x00, 0x00, 0x00, 0x00], // TRANSPARENT
        [0x00, 0x00, 0x00, 0xff], // BLACK
        [0x3e, 0xb8, 0x49, 0xff], // MEDIUM_GREEN
        [0x74, 0xd0, 0x7d, 0xff], // LIGHT_GREEN
        [0x59, 0x55, 0xe0, 0xff], // DARK_BLUE
        [0x80, 0x76, 0xf1, 0xff], // LIGHT_BLUE
        [0xb9, 0x5e, 0x51, 0xff], // DARK_RED
        [0x65, 0xdb, 0xef, 0xff], // CYAN
        [0xdb, 0x65, 0x59, 0xff], // MEDIUM_RED
        [0xff, 0x89, 0x7d, 0xff], // LIGHT_RED
        [0xcc, 0xc3, 0x5e, 0xff], // DARK_YELLOW
        [0xde, 0xd0, 0x87, 0xff], // LIGHT_YELLOW
        [0x3a, 0xa2, 0x41, 0xff], // DARK_GREEN
        [0xb7, 0x66, 0xb5, 0xff], // MAGENTA
        [0xcc, 0xcc, 0xcc, 0xff], // GRAY
        [0xff, 0xff, 0xff, 0xff], // WHITE
    ];

    imports = { 
        wasi_snapshot_preview1: {
            fd_seek: (a) => {},
            fd_write: (a) => {},
            fd_close: (a) => {},
            proc_exit: (a) => {},
        },
        env: { 
            wbe_get_keydown: () => { 
                return this.events.shift() || 0; 
            },
            wbe_set_color: (c) => {
                this.renderer.setColor(c);
            },
            wbe_clear: () => {
                this.renderer.clear();
            },
            wbe_update_buf: (ptr, offset, len) => {
                const buf = new Float32Array(this.exports.memory.buffer, ptr, len);
                this.renderer.updateBuf(buf, offset);
            },
            wbe_draw_buf: (offset, len) => {
                this.renderer.drawBuf(offset, len);
            },
            wbe_draw_lines: (ptr, len) => {
                const buf = new Float32Array(this.exports.memory.buffer, ptr, len);
                this.renderer.drawLines(buf);
            },
            wbe_toggle_scale_factor: () => {
                this.renderer.toggleScaleFactor();
            },
            wbe_send_audiomsg: (msg) => {
                this.audio.sendMessage(msg);
            },
        },
    };

    async main() {
        const mod = await WebAssembly.compileStreaming(fetch(this.name + ".wasm"));
        const inst = await WebAssembly.instantiate(mod, this.imports);
        this.exports = inst.exports;
        this.update = inst.exports.am_update;
        const scale = this.getScaleFactor(this.width, this.height);
        this.renderer = new CanvasRenderer(this.width, this.height, scale);
        const pixelData = this.loadPixelData();
        await this.renderer.loadTexture(this.palette, pixelData);
        this.audio = new AudioSubsystem(this.name);
        await this.audio.init();
        if (this.exports.am_init(this.webgl)) throw new Error("init failed");

        document.addEventListener("keydown", (e) => {
            const ev = this.eventVariants[e.key];

            if (ev) {
                e.preventDefault();

                if (!e.repeat) {
                    this.events.push(ev);
                }
            } 
        });

        const nextFrame = (timestamp) => {
            if (this.update(timestamp)) {
                window.requestAnimationFrame(nextFrame);
            } else {
                this.renderer.clear();
                this.audio.quit();
            }
        };

        window.requestAnimationFrame(nextFrame);
    }

    loadPixelData() {
        const structPtr = this.exports.wbe_load_pixel_data();
        if (!structPtr) throw new Error("null pixel data");
        const struct = new Int32Array(this.exports.memory.buffer, structPtr, 3);
        const dataPtr = struct[0];
        const width = struct[1];
        const height = struct[2];
        const buf = new Uint8Array(this.exports.memory.buffer, dataPtr, width * height);
        return { buf, width, height };
    }

    getScaleFactor(origW, origH) {
        const w = window.visualViewport.width;
        const h = window.visualViewport.height;

        for (let n of [8, 4, 2]) {
            if (w >= origW * n && h >= origH * n) {
                return n;
            }
        }

        return 1;
    }
}

class CanvasRenderer {
    constructor(width, height, scale) {
        const oldCanvas = document.querySelector("canvas");

        if (oldCanvas) {
            document.body.removeChild(oldCanvas);
        }

        const canvas = document.createElement("canvas");
        document.body.appendChild(canvas);
        this.ctx = canvas.getContext("2d", { alpha: false });

        if (!this.ctx) {
            document.body.removeChild(canvas);
            const p = document.createElement("p");
            p.textContent = "Sorry, your browser does not support canvas rendering.";
            document.body.appendChild(p);
            throw new Error("Could not create 2D context.");
        }

        this.offscreen = document.createElement("canvas");
        this.origWidth = width;
        this.origHeight = height;
        this.setScaleFactor(scale);
    }

    async loadTexture(palette, pixelData) {
        const img = this.ctx.createImageData(pixelData.width, pixelData.height);

        for (let i = 0; i < pixelData.buf.length; i++) {
            const j = i * 4;
            const c = pixelData.buf[i];
            img.data[j + 0] = palette[c][0];
            img.data[j + 1] = palette[c][1];
            img.data[j + 2] = palette[c][2];
            img.data[j + 3] = palette[c][3];
        }

        this.texture = await createImageBitmap(img);
        this.cssPalette = palette.map(c => { return `rgb(${c[0]}, ${c[1]}, ${c[2]}, ${c[3]})` });
    }

    setColor(c) {
        this.ctx.fillStyle = this.cssPalette[c];
        this.ctx.strokeStyle = this.cssPalette[c];
    }

    updateBuf(buf, offset) {
        let ctx;

        if (offset === 0) {
            ctx = this.offscreen.getContext("2d");
            ctx.canvas.width = this.origWidth;
            ctx.canvas.height = this.origHeight;
        } else {
            ctx = this.ctx;
        }

        for (let i = 0; i < buf.length; i += 16) {
            const dx = buf[i + 0];
            const sx = buf[i + 2];
            const dy = buf[i + 5];
            const sy = buf[i + 7];
            const dw = buf[i + 8] - dx;
            const dh = buf[i + 9] - dy;
            const sw = buf[i + 10] - sx;
            const sh = buf[i + 11] - sy;
            ctx.drawImage(this.texture, sx, sy, sw, sh, dx, dy, dw, dh);
        }
    }   

    drawBuf(offset, len) {
        if (offset === 0) {
            this.ctx.drawImage(this.offscreen, 0, 0);
        }
    }

    drawLines(buf) {
        this.ctx.beginPath();

        for (let i = 0; i < buf.length; i += 4) {
            const x1 = buf[i + 0];
            const y1 = buf[i + 1];
            const x2 = buf[i + 2];
            const y2 = buf[i + 3];
            this.ctx.moveTo(x1 + .5, y1 + .5);
            this.ctx.lineTo(x2 + .5, y2 + .5);
        }

        this.ctx.stroke();
    }

    toggleScaleFactor() {
        switch (this.scaleFactor) {
            case 1:
                this.setScaleFactor(2);
                break;
            case 2:
                this.setScaleFactor(4);
                break;
            case 4:
                this.setScaleFactor(8);
                break;
            default:
                this.setScaleFactor(1);
        }
    }

    setScaleFactor(sf) {
        this.ctx.canvas.width = this.origWidth * sf;
        this.ctx.canvas.height = this.origHeight * sf;
        this.ctx.setTransform(1, 0, 0, 1, 0, 0);
        this.ctx.scale(sf, sf);
        this.ctx.imageSmoothingEnabled = false;
        this.scaleFactor = sf;
    }

    clear() {
        this.ctx.fillRect(0, 0, this.origWidth, this.origHeight);
    }
}

class AudioSubsystem {
    constructor(name) {
        this.ctx = new AudioContext();
        this.name = name;
    }

    async init() {
        const modName = this.name + "_audio";
        await this.ctx.audioWorklet.addModule(modName + ".js");
        const res = await fetch(modName + ".wasm");
        const wasmSrc = await res.arrayBuffer();
        const options = { 
            numberOfInputs: 0, 
            processorOptions: { wasmSrc } 
        };
        this.worklet = new AudioWorkletNode(this.ctx, modName, options);
        this.worklet.connect(this.ctx.destination);
    }

    sendMessage(msg) {
        this.worklet.port.postMessage(msg);
    }

    quit() {
        this.worklet.disconnect();
        this.ctx.close();
    }
}

new WasmGame("antimatter", 256, 192).main();
