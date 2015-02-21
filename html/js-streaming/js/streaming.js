function connect(address) {
    if ("WebSocket" in window) {
        console.log("Connecting!");

        // open connection
        var ws = new WebSocket(address);

        ws.onmessage = function(evt) {
            var received_msg = evt.data;
            console.log("got msg: " + received_msg);
        };

        ws.onclose = function() {
            console.log("Disconnected.");
        };

        return ws;
    } else {
        alert("WebSocket NOT supported by your Browser!");
    }
}

// ws.send("hello");
