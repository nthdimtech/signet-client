// Load settings with the storage API.

var isChrome = false;

if (isChrome) {
	browser = chrome;
}

const serverUrl = 'ws://localhost:910'

dataSendOnOpen = null;

socket = null;

messageRespond = null;

messageRequest = null;

var tabInfo = new Map([]);

var lastWebsocketMessage = null;
var lastWebsocketMessageInfo = null;
console.log("Starting background script");

chrome.webNavigation['onBeforeNavigate'].addListener(
	function (details) {
	}
);

chrome.webNavigation.onCommitted.addListener(
	function (details) {
		if (details.frameId == 0) {
			console.log("onCommitted:", details.frameId, details.url);
			var info = {messageType: "pageLoaded", url: details.url, pages: new Map([])};
			tabInfo.set(details.tabId, info);
			var data = {messageType: "pageLoaded", url: details.url};
			sendWebsocketMessage(data, {tabId : details.tabId});
		}
	}
);

function createSocket() {
	socket = new WebSocket(serverUrl);

	socket.onmessage = function(event) {
		console.log("WebSocket message received:", JSON.parse(event.data));
		if (messageRespond != null) {
			console.log("Forwarding message to content script");
			messageRespond(event.data);
			messageRespond = null;
			if (messageRequest.method == "selectEntry") {
				var tab = tabInfo.get(messageRequest.tabId);
				console.log("Selecting from tab", tab);
				tab.pages.forEach(function(val, key, map) {
					if (val.hasLoginForm && val.hasUsernameField && val.hasPasswordField) {
						event.data.url = val.url;
						browser.tabs.sendMessage(messageRequest.tabId, event.data);
						return;
					}
				});
				tab.pages.forEach(function(val, key, map) {
					if (val.hasLoginForm) {
						event.data.url = val.url;
						browser.tabs.sendMessage(messageRequest.tabId, event.data);
						return;
					}
				});
				tab.pages.forEach(function(val, key, map) {
					if (val.hasUsernameField || val.hasPasswordField) {
						event.data.url = val.url;
						browser.tabs.sendMessage(messageRequest.tabId, event.data);
					}
				});
			}
		} else if (lastWebsocketMessage != null && lastWebsocketMessage.messageType == "pageLoaded" ) {
			console.log("Got matches", event.data, lastWebsocketMessageInfo.tabId);
			tabInfo.get(lastWebsocketMessageInfo.tabId).pageMatches = JSON.parse(event.data);
		} else {
			console.log("Unexpected websocket message");
		}
	};

	socket.onclose = function(event) {
		console.log("WebSocket closed");
		browser.browserAction.disable();
		messageRespond = null;
		socket = null;
	};

	socket.onopen = function(event) {
		console.log("WebSocket opened");
		browser.browserAction.enable();
		if (dataSendOnOpen != null) {
			console.log("WebSocket sending URL on open", JSON.stringify(dataSendOnOpen));
			socket.send(JSON.stringify(dataSendOnOpen));
			lastWebsocketMessage = dataSendOnOpen;
			dataSendOnOpen = null;
		}
	};

	socket.onerror = function(event) {
		console.log("WebSocket error", event);
	}

}

createSocket();

var sendWebsocketMessage = function (data, info) {
	lastWebsocketMessageInfo = info;
	if (socket != null && socket.readyState == WebSocket.OPEN) {
		lastWebsocketMessage = data;
		socket.send(JSON.stringify(data));
	} else if (socket == null) {
		dataSendOnOpen = data;
		createSocket();
	}
}

browser.runtime.onMessage.addListener(function (req, sender, res) {
	if (req.method == "pageLoaded") {
		console.log("pageLoaded message recieved:", req.data, sender.tab.id);
		tabInfo.get(sender.tab.id).pages.set(req.data.url, req.data);
		return false;
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
		var tabLocated = function(tabA) {
			messageRespond({tabId: tabA[0].id, pageMatches: tabInfo.get(tabA[0].id).pageMatches});
			messageRespond = null;
		};
		if (isChrome) {
			browser.tabs.query({currentWindow: true, active: true}, tabLocated);
		} else {
			var querying = browser.tabs.query({currentWindow: true, active: true});
			querying.then(tabLocated, function () { });
		}
		return true;
	}
});
