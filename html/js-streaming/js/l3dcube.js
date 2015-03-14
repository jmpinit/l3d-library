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

        if(msg == 's') {
            cube.clearToSend = false;
            setTimeout(function() {
                cube.clearToSend = true;
            }, 5000);
        }

        /*if(parseInt(msg) == this.frameSize) { FIXME
            cube.clearToSend = true;
        } else {
            if(!this.clearToSend) {
                setTimeout(function() { cube.clearToSend = true; }, cube.rate);
            }
        }*/
    };
}

function clamp(x, a, b) {
    return Math.max(a, Math.min(x, b));
}

Cube.prototype = {
    changeSize: function(newsize) {
        this.frameSize = newsize;
        this.frameBuffer = new ArrayBuffer(this.frameSize);
        console.log("size changed to ", this.frameSize);
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

    refresh: function() {
        var cube = this;

        //if(this.clearToSend) { FIXME
        if(this.clearToSend && this.ws.bufferedAmount == 0) {
            if(this.onrefresh !== undefined) {
                this.onrefresh(this);
            }

            this.ws.send(this.frameBuffer);
            //this.clearToSend = false;

            setTimeout(function() { cube.refresh(); }, cube.rate);
            //console.log("sent frame");
        } else {
            // check for readiness every 5 millis
            setTimeout(function() { cube.refresh(); }, 5);
        }
    }
}
