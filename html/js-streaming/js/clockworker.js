var interval = null;

function setMyInterval(ms) {
    interval = setInterval(function() {
        self.postMessage("tick");
    }, ms);
}

function clearMyInterval() {
    clearInterval(interval);
    interval = null;
}

self.addEventListener('message', function(e) {
    if (e.data.constructor !== Array)
        throw "message must be an array.";

    switch (e.data[0]) {
        case "set":
            if (e.data.length != 2) throw "needs 1 arg.";

            if (interval !== null)
                clearMyInterval();

            setMyInterval(e.data[1]);
            break;

        case "stop":
            clearMyInterval();
            break;
    };
}, false);
