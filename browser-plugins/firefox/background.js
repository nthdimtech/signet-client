// Load settings with the storage API.
var gettingStoredStats = browser.storage.local.get();

const serverUrl = 'ws://localhost:910'

dataSendOnOpen = null;

socket = null;

messageRespond = null;

function createSocket() {
	socket = new WebSocket(serverUrl);

	socket.onmessage = function(event) {
		console.debug("WebSocket message received:", JSON.parse(event.data));
		if (messageRespond != null) {
			console.debug("Forwarding message to content script");
			messageRespond(event.data);
			messageRespond = null;
		} else {
			console.debug("No response function");
		}
	};

	socket.onclose = function(event) {
		console.debug("WebSocket closed");
		messageRespond = null;
		socket = null;
	};

	socket.onopen = function(event) {
		console.debug("WebSocket opened");
		if (dataSendOnOpen != null) {
			console.debug("WebSocket sending URL on open", JSON.stringify(dataSendOnOpen));
			socket.send(JSON.stringify(dataSendOnOpen));
			dataSendOnOpen = null;
		}
	};

	socket.onerror = function(event) {
		console.debug("WebSocket error", event);
	}

}

chrome.runtime.onMessage.addListener(function (req, sender, res) {
	if (req.method == "page_loaded") {
		console.log("Runtime message recieved:", req.data);
		messageRespond = res;
		if (socket != null && socket.readyState == WebSocket.OPEN) {
			socket.send(JSON.stringify(req.data));
		} else if (socket == null) {
			dataSendOnOpen = req.data;
			createSocket();
		}
		return true;
	}
});

gettingStoredStats.then(results => {
  // Initialize the saved stats if not yet initialized.
	if (!results.stats) {
		results = {
		};
	}
});
