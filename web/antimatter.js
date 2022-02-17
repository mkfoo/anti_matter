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

const EVENT_VALUES = {
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

class CanvasRenderer {
    constructor() {
        this.width = 256;
        this.height = 192;
        this.canvas = document.getElementById("canvas");
        this.ctx = canvas.getContext("2d");
        this.ctx.imageSmoothingEnabled = false;
        this.texture = null;
        this.cssColors = COLORS.map(c => { return `rgb(${c[0]}, ${c[1]}, ${c[2]}, ${c[3]})` });
        this.setScaleFactor(2);
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

        this.texture = await createImageBitmap(output);
    }

    blitToCanvas(sx, sy, sw, sh, dx, dy) {
        if (this.texture) {
            this.ctx.drawImage(this.texture, sx, sy, sw, sh, dx, dy, sw, sh);
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

    quit() {
        this.ctx.clearRect(0, 0, 256, 192);
    }
}

class AudioSubsystem {
    constructor() {
        this.ctx = new AudioContext();
        this.worklet = null;
    }

    async init() {
        await this.ctx.audioWorklet.addModule("antimatter_audio.js");
        const res = await fetch("antimatter_audio.wasm");
        const wasmSrc = await res.arrayBuffer();
        const options = { 
            numberOfInputs: 0, 
            processorOptions: { wasmSrc } 
        };
        this.worklet = new AudioWorkletNode(this.ctx, "antimatter_audio", options);
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

let renderer = new CanvasRenderer();
let update;
let memory;
let audio;
let events = [];

const importObj = { 
    wasi_snapshot_preview1: {
        proc_exit: (a) => {},
    },

    env: { 
        wbe_get_keydown: () => { return events.shift() || 0; },
        wbe_upload_texture: (ptr, width, height) => {
            const a = new Int32Array(memory, ptr, 2);
            const b = new Uint8Array(memory, a[0], a[1]);
            renderer.loadTexture(b, width, height);
        },
        wbe_blit_to_canvas: (sx, sy, sw, sh, dx, dy) => {
            renderer.blitToCanvas(sx, sy, sw, sh, dx, dy);
        },
        wbe_fill_rect: (x, y, w, h, c) => {
            renderer.fillRect(x, y, w, h, c);
        },
        wbe_draw_line: (x1, y1, x2, y2, c) => {
            renderer.drawLine(x1, y1, x2, y2, c);
        },
        wbe_toggle_scale_factor: () => {
            renderer.toggleScaleFactor();
        },
        wbe_send_audiomsg: (msg) => {
            audio.sendMessage(msg);
        },
    },
};

function catchEvent(e) {
    const val = EVENT_VALUES[e.key];

    if (val) {
        e.preventDefault();

        if (!e.repeat) {
            events.push(val);
        }
    } 
}

function nextFrame(timestamp) {
    if (update(timestamp)) {
        window.requestAnimationFrame(nextFrame);
    } else {
        renderer.quit();
        audio.quit();
    }
}

async function run() {
    const mod = await WebAssembly.compileStreaming(fetch("antimatter.wasm"));
    const inst = await WebAssembly.instantiate(mod, importObj);
    update = inst.exports.am_update;
    memory = inst.exports.memory.buffer;
    audio = new AudioSubsystem();
    await audio.init();
    const err = inst.exports.am_init();
    if (err) throw new Error("init failed");
    document.addEventListener("keydown", catchEvent);
    window.requestAnimationFrame(nextFrame);
}

export default run;
