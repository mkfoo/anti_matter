let wasm;

const importObj = { 
    wasi_snapshot_preview1: {
        args_get: (a, b) => { return 2; },
        args_sizes_get: (a, b) => { return 2; },
        fd_close: (a) => { return 2; },
        fd_fdstat_get: (a, b) => { return 2; },
        fd_seek: (a, b, c, d) => { return 2; },
        fd_write: (fd, ptr, arg2, arg3) => { 
            const str = wasm.importString(ptr);
            console.log(str);
            return 2;
        },
        proc_exit: (a) => {},
    },

    env: { 
        wbe_get_keydown: () => { return wasm.getKeyDown(); },
        wbe_upload_texture: (ptr, width, height) => {
            const bytes = wasm.importBytes(ptr);
            wasm.renderer.loadTexture(bytes, width, height);
        },
        wbe_blit_to_canvas: (sx, sy, sw, sh, dx, dy) => {
            wasm.renderer.blitToCanvas(sx, sy, sw, sh, dx, dy);
        },
        wbe_fill_rect: (x, y, w, h, c) => {
            wasm.renderer.fillRect(x, y, w, h, c);
        },
        wbe_draw_line: (x1, y1, x2, y2, c) => {
            wasm.renderer.drawLine(x1, y1, x2, y2, c);
        },
        wbe_toggle_scale_factor: () => {
            wasm.renderer.toggleScaleFactor();
        }
    },
};

const COLORS = [
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

class CanvasRenderer {
    constructor() {
        this.width = 256;
        this.height = 192;
        this.canvas = document.getElementById("canvas");
        this.ctx = canvas.getContext("2d");
        this.ctx.imageSmoothingEnabled = false;
        this.textures = [];
        this.cssColors = COLORS.map(c => { return `rgb(${c[0]}, ${c[1]}, ${c[2]}, ${c[3]})` });
        this.scaleFactor = 1;
    }

    async loadTexture(input, width, height) {
        const output = this.ctx.createImageData(width, height);

        for (let i = 0; i < input.length; i++) {
            const j = i * 4;
            const c = input[i];
            output.data[j + 0] = COLORS[c][0];
            output.data[j + 1] = COLORS[c][1];
            output.data[j + 2] = COLORS[c][2];
            output.data[j + 3] = COLORS[c][3];
        }

        const bitmap = await createImageBitmap(output);
        this.textures.push(bitmap);
    }

    blitToCanvas(sx, sy, sw, sh, dx, dy) {
        if (this.textures.length) {
            this.ctx.drawImage(this.textures[0], sx, sy, sw, sh, dx, dy, sw, sh);
        }
    }   

    fillRect(x, y, w, h, c) {
        this.ctx.fillStyle = this.cssColors[c];
        this.ctx.fillRect(x, y, w, h);
    }

    drawLine(x1, y1, x2, y2, c) {
        this.ctx.beginPath();
        this.ctx.strokeStyle = this.cssColors[c];
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
        this.canvas.width = this.width * sf;
        this.canvas.height = this.height * sf;
        this.ctx.setTransform(1, 0, 0, 1, 0, 0);
        this.ctx.scale(sf, sf);
        this.ctx.imageSmoothingEnabled = false;
        this.scaleFactor = sf;
    }
}

const EVENT_TYPES = {
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
};

class WasmBackend {
    constructor(inst) {
        this.init = inst.exports.am_init;
        this.update = inst.exports.am_update;
        this.mem = inst.exports.memory.buffer;
        this.events = [];
        this.renderer = new CanvasRenderer();
    }

    importBytes(ptr) {
        const a = new Int32Array(this.mem, ptr, 2);
        return new Uint8Array(this.mem, a[0], a[1]);
    }

    importString(ptr) {
        const bytes = this.importBytes(ptr);

        try {
            const str = new TextDecoder("utf-8").decode(bytes);
            return str;
        } catch (err) {
            console.log(`UTF-8 error: ${err.message}`);
            return undefined;
        }
    }

    addKeyDown(e) {
        if (!e.repeat) {
            const v = EVENT_TYPES[e.key];
            if (v) this.events.push(v);
        }
    }

    getKeyDown() {
        return this.events.shift() || 0; 
    }
}

function nextFrame(timestamp) {
    if (wasm.update(timestamp)) {
        window.requestAnimationFrame(nextFrame);
    }
}

async function run() {
    try {
        const mod = await WebAssembly.compileStreaming(fetch("./antimatter.wasm"));
        const inst = await WebAssembly.instantiate(mod, importObj);
        wasm = new WasmBackend(inst);
        const err = wasm.init();
        if (err) throw new Error("init failed");
        document.addEventListener("keydown", (e) => { wasm.addKeyDown(e); });
        window.requestAnimationFrame(nextFrame);
    } catch (err) {
        console.log(err.message);
    }
}

export default run;
