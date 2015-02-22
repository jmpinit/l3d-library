function connect(address) {
    if ("WebSocket" in window) {
        console.log("Connecting!");

        // open connection
        var ws = new WebSocket(address);

        ws.onclose = function() {
            console.log("Disconnected.");
        };

        return ws;
    } else {
        alert("WebSocket NOT supported by your Browser!");
    }
}

// ws.send("hello");
