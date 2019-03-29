// Load settings with the storage API.
var gettingStoredStats = browser.storage.local.get();

const serverUrl = 'ws://localhost:910'

dataSendOnOpen = null;

socket = null;

messageRespond = null;

messageRequest = null;

var pageInfo = new Map([]);

function createSocket() {
	socket = new WebSocket(serverUrl);

	socket.onmessage = function(event) {
		console.debug("WebSocket message received:", JSON.parse(event.data));
		if (messageRespond != null) {
			console.debug("Forwarding message to content script");
			messageRespond(event.data);
			if (messageRequest.method == "pageLoaded") {
				pageInfo.get(messageRequest.data.tabId).pageMatches = JSON.parse(event.data);
			}
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

var sendWebsocketMessage = function (data) {
	if (socket != null && socket.readyState == WebSocket.OPEN) {
		socket.send(JSON.stringify(data));
	} else if (socket == null) {
		dataSendOnOpen = data;
		createSocket();
	}
}

chrome.runtime.onMessage.addListener(function (req, sender, res) {
	if (req.method == "pageLoaded") {
		console.log("pageLoaded message recieved:", req.data, sender.tab.id);
		req.data.tabId = sender.tab.id;
		pageInfo.set(req.data.tabId, req.data);
		messageRespond = res;
		messageRequest = req;
		sendWebsocketMessage(req.data);
		return true;
	} else if (req.method == "selectEntry") {
		console.log("selectEntry message recieved:", req.data);
		messageRespond = res;
		messageRequest = req;
		sendWebsocketMessage(req.data);
		return true;
	} else if (req.method == "popupLoaded") {
		console.log("popupLoaded message recieved:", req.data);
		messageRespond = res;
		messageRequest = req;
		var querying = browser.tabs.query({currentWindow: true, active: true});
		querying.then(
			function(tabA) {
				console.log("Current tab", tabA[0].id, pageInfo.get(tabA[0].id).pageMatches);
				messageRespond(pageInfo.get(tabA[0].id).pageMatches);
				messageRespond = null;
			}
			,
			function () {
			});
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
