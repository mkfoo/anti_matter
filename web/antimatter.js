class WasmGame {
    constructor(name, width, height, webgl) {
        this.name = name;
        this.width = width;
        this.height = height;
        this.events = [];
        this.webgl = webgl;
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

        if (this.webgl) {
            try {
                this.renderer = new WebGLRenderer(this.width, this.height, scale);
            } catch (err) {
                console.log(err.message + "Falling back on canvas rendering.");
                this.webgl = 0;
            }
        }

        if (!this.webgl) {
            this.renderer = new CanvasRenderer(this.width, this.height, scale);
        }

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

class WebGLRenderer {
    constructor(width, height, scale) {
        const vsSource = `
            attribute vec4 aInput;
            varying highp vec2 vSrcXY;

            uniform mat4 uSrcMatrix;
            uniform mat4 uDstMatrix; 

            void main(void) {
                vSrcXY = (uSrcMatrix * vec4(aInput.z, aInput.w, 0.0, 1.0)).xy;
                gl_Position = uDstMatrix * vec4(aInput.x, aInput.y, 0.0, 1.0);
            }
        `;

        const fsSource = `
            varying highp vec2 vSrcXY;
            uniform sampler2D uSampler;

            void main(void) {
                gl_FragColor = texture2D(uSampler, vSrcXY);
            }
        `;
       
        const canvas = document.createElement("canvas");
        document.body.appendChild(canvas);
        const attrs = {
            alpha: false,
            antialias: false,
            depth: false,
            stencil: false,
        };
        const gl = canvas.getContext("webgl", attrs);

        if (!gl) {
            throw new Error("Could not create WebGL context.");
        }

        this.gl = gl;
        this.color = 1;
        this.origWidth = width;
        this.origHeight = height;
        this.setScaleFactor(scale);
        this.program = this.initProgram(vsSource, fsSource);
        this.bufSize = 2 ** 16;
        this.elemSize = 4;

        this.attributes = {
            aInput: gl.getAttribLocation(this.program, "aInput"),
        };

        this.uniforms = {
            uSrcMatrix: gl.getUniformLocation(this.program, "uSrcMatrix"),
            uDstMatrix: gl.getUniformLocation(this.program, "uDstMatrix"),
            uSampler: gl.getUniformLocation(this.program, "uSampler"),
        };

        this.buffers = this.initBuffers();
    }

    initProgram(vsSource, fsSource) {
        const gl = this.gl;
        const vs = this.loadShader(gl.VERTEX_SHADER, vsSource);
        const fs = this.loadShader(gl.FRAGMENT_SHADER, fsSource);
        const program = gl.createProgram();
        gl.attachShader(program, vs);
        gl.attachShader(program, fs);
        gl.linkProgram(program);

        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
            throw new Error(gl.getProgramInfoLog(program));
        }

        return program;
    }

    loadShader(type, source) {
        const gl = this.gl;
        const shader = gl.createShader(type);
        gl.shaderSource(shader, source);
        gl.compileShader(shader);

        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            throw new Error(gl.getShaderInfoLog(shader));
        }

        return shader;
    }

    initBuffers() {
        const gl = this.gl;
        const inputBuf = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, inputBuf);
        gl.bufferData(gl.ARRAY_BUFFER, this.bufSize, gl.DYNAMIC_DRAW);
        gl.vertexAttribPointer(this.attributes.aInput, this.elemSize, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(this.attributes.aInput);
        return { inputBuf };
    }

    async loadTexture(palette, pixelData) {
        const gl = this.gl;
        const len = pixelData.buf.length;
        const tw = pixelData.width;
        const th = pixelData.height;
        const rgba = new Uint8Array(len * 4);

        for (let i = 0; i < len; i++) {
            const j = i * 4;
            const c = pixelData.buf[i];
            rgba[j + 0] = palette[c][0];
            rgba[j + 1] = palette[c][1];
            rgba[j + 2] = palette[c][2];
            rgba[j + 3] = palette[c][3];
        }

        const texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texture);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, tw, th, 0, gl.RGBA, gl.UNSIGNED_BYTE, rgba);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.activeTexture(gl.TEXTURE0);
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
        gl.clearDepth(1.0); 
        this.initMatrices(tw, th);
        this.palette = palette;
        return texture;
    }

    initMatrices(textureW, textureH) {
        const gl = this.gl;
        const sx = 1 / textureW;
        const sy = 1 / textureH;
        const dx = -2 * 1 / -this.origWidth;
        const dy = -2 * 1 / this.origHeight;

        const srcMatrix = [sx,  0,  0,  0,
                            0, sy,  0,  0,
                            0,  0,  1,  0,
                            0,  0,  0,  1];

        const dstMatrix = [dx,  0,  0,  0,
                            0, dy,  0,  0,
                            0,  0, -1,  0,
                           -1,  1,  0,  1];

        gl.useProgram(this.program);
        gl.uniformMatrix4fv(this.uniforms.uSrcMatrix, false, srcMatrix);
        gl.uniformMatrix4fv(this.uniforms.uDstMatrix, false, dstMatrix);
    }

    updateBuf(buf, offset) {
        const gl = this.gl;
        gl.useProgram(this.program);
//        gl.bindBuffer(gl.ARRAY_BUFFER, this.buffers.inputBuf);
        gl.bufferSubData(gl.ARRAY_BUFFER, offset * this.elemSize, buf);
    }

    drawBuf(offset, len) {
        const gl = this.gl;
        gl.useProgram(this.program);
//        gl.bindBuffer(gl.ARRAY_BUFFER, this.buffers.inputBuf);
        gl.drawArrays(gl.TRIANGLE_STRIP, offset / this.elemSize, len / this.elemSize);
    }

    drawLines(buf) {
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

    setColor(c) {
        const gl = this.gl;
        const r = this.palette[c][0] / 255;
        const g = this.palette[c][1] / 255;
        const b = this.palette[c][2] / 255;
        const a = this.palette[c][3] / 255;
        gl.clearColor(r, g, b, a); 
    }

    setScaleFactor(sf) {
        const gl = this.gl;
        gl.canvas.width = this.origWidth * sf;
        gl.canvas.height = this.origHeight * sf;
        gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
        this.scaleFactor = sf;
    }

    clear() {
        const gl = this.gl;
        gl.clear(gl.COLOR_BUFFER_BIT);
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

new WasmGame("antimatter", 256, 192, 0).main();
