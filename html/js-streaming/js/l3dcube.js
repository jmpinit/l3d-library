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
    this.clearToSend = true;
    this.rate = 1000;
    this.size = 8; // TODO support 16^3
    this.frameSize = 512;

    this.frameBuffer = new ArrayBuffer(this.frameSize);

    // open connection
    this.ws = new WebSocket(address);
    console.log("Connecting!");

    var cube = this;

    this.ws.onclose = function() {
        if(cube.onclose !== undefined) {
            cube.onclose(cube);
        }
    };

    this.ws.onopen = function() {
        if(cube.onopen !== undefined) {
            cube.onopen(cube);
        }

        cube.refresh();
    };

    this.ws.onmessage = function(evt) {
        var msg = evt.data;
        console.log("got msg: " + msg);

        if(parseInt(msg) == this.frameSize) {
            cube.clearToSend = true;
        } else {
            if(!this.clearToSend) {
                setTimeout(function() { cube.clearToSend = true; }, cube.rate);
            }
        }
    };
}

function clamp(x, a, b) {
    return Math.max(a, Math.min(x, b));
}

// compresses a Uint8Array using LZW
// returns compressed Uint8Array
function lzwCompress(uncompressed) {
    "use strict";

    function arrayToString(arr) {
        return arr.join();
    }

    function getWidth(size) {
        return Math.ceil(Math.log2(size));
    }

    function bits(value) {
        return value.toString(2).split('').map(function(digit) {
            return digit == "1"? 1 : 0;
        });
    }

    // build dictionary

    var dictionary = new Map();

    for(var i = 0; i < 256; i++) {
        dictionary.set(arrayToString([i]), dictionary.size);
    }

    var codeWidth = getWidth(dictionary.size);
    var bitIndex = 0;

    // compress

    var compressed = [];
    var word = [];

    for(var i = 0; i < uncompressed.length; i++) {
        var v = uncompressed[i];
        var newWord = word.concat([v]);
        var newWordKey = arrayToString(newWord);

        if(dictionary.has(newWordKey)) {
            word = newWord;
        } else {
            var code = dictionary.get(arrayToString(word))

            // FIXME size of bits(code) must only increase
            compressed = compressed.concat(bits(code));

            // add to dictionary
            dictionary.set(newWordKey, dictionary.size);

            // update code width
            codeWidth = getWidth(dictionary.size);

            word = [v];
        }
    }

    if(word.length != 0) {
        var code = dictionary.get(arrayToString(word));
        compressed = compressed.concat(bits(code));
    }

    var buffer = new Uint8Array(compressed.length / 8);

    for(var i = 0; i < buffer.length; i++) {
        var value = 0;

        for(var k = 7; k >= 0; k--) {
            value += Math.pow(2, k) * compressed[i * 8 + k];
        }

        buffer[i] = value;
    }

    return buffer;
}

function lzwDecompress(compressed) {
    "use strict";

    // build dictionary

    var dictionary = new Map();

    for(var i = 0; i < 256; i++) {
        dictionary.set(i, [dictionary.size]);
    }

    // decompress

    var word = [compressed[0]];
    var uncompressed = word;

    for(var i = 1; i < compressed.length; i++) {
        var k = compressed[i];

        var entry;
        if(dictionary.has(k)) {
            entry = dictionary.get(k);
        } else if(k === dictionary.size) {
            entry = word.concat([word[0]]);
        } else {
            throw "Decompression failed. Invalid value.";
        }

        uncompressed = uncompressed.concat(entry);

        var newEntry = word.concat([entry[0]]);
        dictionary.set(dictionary.size, newEntry);

        word = entry;
    }

    return Uint8Array.from(uncompressed);
}

Cube.prototype = {
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

    refresh: function() {
        var cube = this;

        if(this.clearToSend) {
            if(this.onrefresh !== undefined) {
                this.onrefresh(this);
            }

            this.ws.send(this.frameBuffer);
            this.clearToSend = false;

            setTimeout(function() { cube.refresh(); }, cube.rate);
            console.log("sent frame");
        } else {
            // check for readiness every 5 millis
            setTimeout(function() { cube.refresh(); }, 5);
        }
    }
}
