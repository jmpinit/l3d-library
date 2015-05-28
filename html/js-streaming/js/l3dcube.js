var testTimer = 0;

function clamp(x, a, b) {
    return Math.max(a, Math.min(x, b));
}

var makeCRCTable = function(){
    var c;
    var crcTable = [];
    for(var n =0; n < 256; n++){
        c = n;
        for(var k =0; k < 8; k++){
            c = ((c&1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
        }
        crcTable[n] = c;
    }
    return crcTable;
}

var crc32 = function(buf, len) {
    var crcTable = window.crcTable || (window.crcTable = makeCRCTable());
    var crc = 0 ^ (-1);

    var bufView = new Uint8Array(buf);
    for (var i = 0; i < bufView.length && i < len; i++ ) {
        crc = (crc >>> 8) ^ crcTable[(crc ^ bufView[i]) & 0xFF];
    }

    return (crc ^ (-1)) >>> 0;
};

function Color(r, g, b) {
    this.r = r;
    this.g = g;
    this.b = b;
}

function Point(x, y) {
    this.x = x;
    this.y = y;
}

function Cube(address) {
    this.address = address;

    this.timeout = 1000; // reconnect after no reply in this # of ms
    this.clearToSend = false;
    this.timeOfLastReply = null; // ms since last response
    this.waiting = false; // true if waiting for ack
    
    this.rate = 1000;
    this.size = 8; // TODO support 16^3
    this.frameSize = 512 + 4;

    this.frameBuffer = new ArrayBuffer(this.frameSize);

    this.stream();
}

Cube.prototype = {
    /*  Start transmitting the frame buffer to the cube */
    stream: function() {
        if (window.Worker) {
            this._clock = new Worker("js/clockworker.js");

            this._clock.onerror = function(err) {
                console.log("cube worker error!", err.message);
            };

            var cube = this;
            this._clock.addEventListener('message', function(e) {
                testTimer += 1;

                if (e.data === "tick") {
                    if (cube.clearToSend && cube.ws.bufferedAmount == 0) {
                        // give user chance to modify frame
                        if(cube.onrefresh !== undefined) {
                            cube.onrefresh(cube);
                        }

                        // add crc32
                        var crc = crc32(cube.frameBuffer, 512);
                        var bufView = new Uint8Array(cube.frameBuffer);
                        bufView[512] = (crc >> 24) & 0xff;
                        bufView[513] = (crc >> 16) & 0xff;
                        bufView[514] = (crc >> 8) & 0xff;
                        bufView[515] = crc & 0xff;

                        cube.ws.send(cube.frameBuffer);
                        cube.clearToSend = false; // must get reply before sending again

                        cube._clock.postMessage(["set", cube.rate]);
                        document.title = "sent " + testTimer; // FIXME
                    } else {
                        // try to reconnect if timed out
                        if (cube.timeOfLastReply != null && Date.now() - cube.timeOfLastReply > cube.timeout) {
                            cube.timeOfLastReply = null;
                            cube.ws.close();
                        }
                    }
                }
            });
        } else {
            alert("web workers not supported.");
            console.log("web workers not supported.");
        }

        // open connection
        this.ws = new WebSocket(this.address);
        console.log("Connecting!");

        var cube = this;

        this.ws.onclose = function() {
            cube.clearToSend = false;
            cube._clock.postMessage(["stop"]);

            if(cube.onclose !== undefined) {
                cube.onclose(cube);
            }

            // try to reconnect
            cube.stream();
        };

        this.ws.onopen = function() {
            if(cube.onopen !== undefined) {
                cube.onopen(cube);
            }

            console.log("connected!");
            
            // start streaming
            cube.clearToSend = true;
            cube._clock.postMessage(["set", cube.rate]);
        };

        this.ws.onmessage = function(evt) {
            var msg = evt.data;
            console.log("got msg: " + msg);

            if(parseInt(msg) == cube.frameSize) {
                cube.clearToSend = true;
                cube.timeOfLastReply = Date.now();
            } else {
                // TODO reconnect, because something is wrong
            }
        };
    },

    stop: function() {
        this._clock.postMessage(["stop"]);
    },

    setVoxel: function(x, y, z, r, g, b) {
        x = Math.floor(x);
        y = Math.floor(y);
        z = Math.floor(z);

        r = Math.floor(clamp(r, 0, 255));
        g = Math.floor(clamp(g, 0, 255));
        b = Math.floor(clamp(b, 0, 255));

        if(x >= 0 && y >= 0 && z >= 0 && x < this.size && y < this.size && z < this.size) {
            var index = (z*64) + (x*8) + y;
            var frameView = new Uint8Array(cube.frameBuffer);
            frameView[index] = ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6);
        }
    },

    background: function(r, g, b) {
        r = Math.floor(clamp(r, 0, 255));
        g = Math.floor(clamp(g, 0, 255));
        b = Math.floor(clamp(b, 0, 255));

        for(var x = 0; x < this.size; x++) {
            for(var y = 0; y < this.size; y++) {
                for(var z = 0; z < this.size; z++) {
                    this.setVoxel(x, y, z, r, g, b);
                }
            }
        }
    },

    onopen: function() {},
    onclose: function() {},
    onrefresh: function() {},
}
