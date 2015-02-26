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

    /*getVoxel: function(x, y, z) {
        var index = (z * this.size * this.size) + (x * this.size) + y;
        // TODO
    },*/

    /*line: function(x1, y1, z1, x2, y2, z2, col) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        int dz = z2 - z1;
        int x_inc = (dx < 0) ? -1 : 1;
        int l = abs(dx);
        int y_inc = (dy < 0) ? -1 : 1;
        int m = abs(dy);
        int z_inc = (dz < 0) ? -1 : 1;
        int n = abs(dz);
        int dx2 = l << 1;
        int dy2 = m << 1;
        int dz2 = n << 1;

        if((l >= m) && (l >= n)) {
            int err_1 = dy2 - l;
            int err_2 = dz2 - l;

            for(int i = 0; i < l; i++) {
                this->setVoxel(currentPoint, col);

                if(err_1 > 0) {
                    currentPoint.y += y_inc;
                    err_1 -= dx2;
                }

                if(err_2 > 0) {
                    currentPoint.z += z_inc;
                    err_2 -= dx2;
                }

                err_1 += dy2;
                err_2 += dz2;
                currentPoint.x += x_inc;
            }
        } else if((m >= l) && (m >= n)) {
            int err_1 = dx2 - m;
            int err_2 = dz2 - m;

            for(int i = 0; i < m; i++) {
                this->setVoxel(currentPoint, col);

                if(err_1 > 0) {
                    currentPoint.x += x_inc;
                    err_1 -= dy2;
                }

                if(err_2 > 0) {
                    currentPoint.z += z_inc;
                    err_2 -= dy2;
                }

                err_1 += dx2;
                err_2 += dz2;
                currentPoint.y += y_inc;
            }
        } else {
            int err_1 = dy2 - n;
            int err_2 = dx2 - n;

            for(int i = 0; i < n; i++) {
                this->setVoxel(currentPoint, col);

                if(err_1 > 0) {
                    currentPoint.y += y_inc;
                    err_1 -= dz2;
                }

                if(err_2 > 0) {
                    currentPoint.x += x_inc;
                    err_2 -= dz2;
                }

                err_1 += dy2;
                err_2 += dx2;
                currentPoint.z += z_inc;
            }
        }

        this->setVoxel(currentPoint, col);
    },*/

    /*sphere: function(p, r, col) {
        for(int dx = -r; dx <= r; dx++) {
            for(int dy = -r; dy <= r; dy++) {
                for(int dz = -r; dz <= r; dz++) {
                    if(sqrt(dx*dx + dy*dy + dz*dz) <= r) {
                        setVoxel(x + dx, y + dy, z + dz, col);
                    }
                }
            }
        }
    },*/

    /*shell: function(x, y, z, r, col) {
        for(int i = 0; i <= 2*r; i++) {
            int dy = i - r;
            int lr = sqrt((float)(i*(2*r-i)));
            this->emptyFlatCircle(x, y + dy, z, lr, col);
        }
    },*/

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

    /*colorMap: function(val, min, max) {
        const float range = 1024;
        val = range * (val-min) / (max-min);

        Color colors[6];

        colors[0].red = 0;
        colors[0].green = 0;
        colors[0].blue = this->maxBrightness;

        colors[1].red = 0;
        colors[1].green = this->maxBrightness;
        colors[1].blue = this->maxBrightness;

        colors[2].red = 0;
        colors[2].green = this->maxBrightness;
        colors[2].blue = 0;

        colors[3].red = this->maxBrightness;
        colors[3].green = this->maxBrightness;
        colors[3].blue = 0;

        colors[4].red = this->maxBrightness;
        colors[4].green = 0;
        colors[4].blue = 0;

        colors[5].red = this->maxBrightness;
        colors[5].green = 0;
        colors[5].blue = this->maxBrightness;

        if(val <= range/6)
            return lerpColor(colors[0], colors[1], val, 0, range/6);
        else if(val <= 2 * range / 6)
            return(lerpColor(colors[1], colors[2], val, range / 6, 2 * range / 6));
        else if(val <= 3 * range / 6)
            return(lerpColor(colors[2], colors[3], val, 2 * range / 6, 3*range / 6));
        else if(val <= 4 * range / 6)
            return(lerpColor(colors[3], colors[4], val, 3 * range / 6, 4*range / 6));
        else if(val <= 5 * range / 6)
            return(lerpColor(colors[4], colors[5], val, 4 * range / 6, 5*range / 6));
        else
            return(lerpColor(colors[5], colors[0], val, 5 * range / 6, range));
    },*/

    /*lerpColor: function(a, b, val, min, max) {
        int red = a.red + (b.red-a.red) * (val-min) / (max-min);
        int green = a.green + (b.green-a.green) * (val-min) / (max-min);
        int blue = a.blue + (b.blue-a.blue) * (val-min) / (max-min);

        return Color(red, green, blue);
    },*/

    onopen: function() {},
    onclose: function() {},
    onrefresh: function() {},

    refresh: function() {
        if(this.onrefresh !== undefined) {
            this.onrefresh(this);
        }
        
        var cube = this;

        if(this.clearToSend) {
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
