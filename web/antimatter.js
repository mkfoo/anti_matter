class WasmGame {
    constructor(name) {
        this.name = name;
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
            proc_exit: (a) => {},
        },
        env: { 
            wbe_get_keydown: () => { 
                return this.events.shift() || 0; 
            },
            wbe_texture_copy: (sx, sy, sw, sh, dx, dy) => {
                this.renderer.textureCopy(sx, sy, sw, sh, dx, dy);
            },
            wbe_fill_rect: (x, y, w, h, c) => {
                this.renderer.fillRect(x, y, w, h, c);
            },
            wbe_draw_line: (x1, y1, x2, y2, c) => {
                this.renderer.drawLine(x1, y1, x2, y2, c);
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
        this.memory = inst.exports.memory.buffer;
        this.renderer = new CanvasRenderer(this.palette);

        const pixelData = this.loadPixelData();
        await this.renderer.loadTexture(pixelData);

        this.audio = new AudioSubsystem(this.name);
        await this.audio.init();

        if (this.exports.am_init()) throw new Error("init failed");

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
                this.renderer.quit();
                this.audio.quit();
            }
        };

        window.requestAnimationFrame(nextFrame);
    }

    loadPixelData() {
        const structPtr = this.exports.wbe_load_pixel_data();
        if (!structPtr) throw new Error("null pixel data");
        const struct = new Int32Array(this.memory, structPtr, 3);
        const dataPtr = struct[0];
        const width = struct[1];
        const height = struct[2];
        const input = new Uint8Array(this.memory, dataPtr, width * height);
        const buf = new Uint8Array(input.length * 4);

        for (let i = 0; i < input.length; i++) {
            const j = i * 4;
            const c = input[i];
            buf[j + 0] = this.palette[c][0];
            buf[j + 1] = this.palette[c][1];
            buf[j + 2] = this.palette[c][2];
            buf[j + 3] = this.palette[c][3];
        }
        
        return { buf, width, height };
    }
}

class CanvasRenderer {
    constructor(palette) {
        const canvas = document.getElementById("canvas"); 
        this.origWidth = canvas.width;
        this.origHeight = canvas.height;
        this.ctx = canvas.getContext("2d");
        this.ctx.imageSmoothingEnabled = false;
        this.cssPalette = palette.map(c => { return `rgb(${c[0]}, ${c[1]}, ${c[2]}, ${c[3]})` });
        this.setScaleFactor(2);
    }

    async loadTexture(pixelData) {
        const img = this.ctx.createImageData(pixelData.width, pixelData.height);
        img.data.set(pixelData.buf);
        this.texture = await createImageBitmap(img);
    }

    textureCopy(sx, sy, sw, sh, dx, dy) {
        this.ctx.drawImage(this.texture, sx, sy, sw, sh, dx, dy, sw, sh);
    }   

    fillRect(x, y, w, h, c) {
        this.ctx.fillStyle = this.cssPalette[c];
        this.ctx.fillRect(x, y, w, h);
    }

    drawLine(x1, y1, x2, y2, c) {
        this.ctx.beginPath();
        this.ctx.strokeStyle = this.cssPalette[c];
        this.ctx.moveTo(x1 + .5, y1 + .5);
        this.ctx.lineTo(x2 + .5, y2 + .5);
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

    quit() {
        this.ctx.clearRect(0, 0, 256, 192);
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

new WasmGame("antimatter").main();
