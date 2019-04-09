// Load settings with the storage API.

var isChrome = false;

if (typeof browser === 'undefined') {
	isChrome = true;
	browser = chrome;
}

const serverUrl = 'ws://localhost:10910'

dataSendOnOpen = null;

socket = null;

messageRespond = null;

messageRequest = null;

var tabInfo = new Map([]);
var activeTabId = null;

var initTabInfo = function (tabId, tabUrl) {
	tabInfo.set(tabId, {url: tabUrl, pages: new Map()});
};

var updateBrowserActionStatus = function()
{
	if (socket == null || socket.readyState != WebSocket.OPEN) {
		browser.browserAction.setIcon({path: "icons/icon-32.png"});
		browser.browserAction.disable();
	} else {
		browser.browserAction.enable();
		var activeTabInfo = tabInfo.get(activeTabId);
		if (activeTabInfo == null) {
			browser.browserAction.setIcon({path: "icons/icon-32.png"});
		} else {
			if (activeTabInfo.pageMatches != null && activeTabInfo.pageMatches.length > 0) {
				//Show at least white ring
				if (activeTabInfo.pages != null) {
					var foundLoginForm = false;
					console.log("pages", activeTabInfo.pages);
					activeTabInfo.pages.forEach(function(val, key, map) {
						if (val.hasLoginForm) {
							foundLoginForm = true;
						}
					});
					if (foundLoginForm) {
						browser.browserAction.setIcon({path: "icons/icon-32-login.png"});
					} else {
						browser.browserAction.setIcon({path: "icons/icon-32-matches.png"});
					}
				} else {
					browser.browserAction.setIcon({path: "icons/icon-32-matches.png"});
				}
			} else {
				browser.browserAction.setIcon({path: "icons/icon-32.png"});
			}
		}
	}
};

var activeTabChanged = function (tabId) {
	console.log("Active tab changed ", tabId);
	activeTabId = tabId;
	if (activeTabId != null) {
		if (tabInfo.get(activeTabId) == null) {
			initTabInfo(activeTabId, "");

			var gotTab = function(tab) {
				tabInfo.get(activeTabId).url = tab.url;
				var data = {messageType: "pageLoaded", url: tab.url};
				sendWebsocketMessage(data, {tabId : tab.id});
			}

			if (isChrome) {
				browser.tabs.get(activeTabId, gotTab);
			} else {
				var getting = browser.tabs.get(activeTabId);
				getting.then(gotTab);
			}
		} else {
			browser.tabs.sendMessage(activeTabId, JSON.stringify({method : "loadPage"}));
		}
	} 
	updateBrowserActionStatus();
}

browser.tabs.onActivated.addListener(function (activeInfo) {
	activeTabChanged(activeInfo.tabId);
});

var initialTabLocated = function(details) {
	activeTabChanged(details.tabId);
};

if (isChrome) {
	browser.tabs.query({currentWindow: true, active: true}, initialTabLocated);
} else {
	var querying = browser.tabs.query({currentWindow: true, active: true});
	querying.then(initialTabLocated);
}

var lastWebsocketMessage = null;
var lastWebsocketMessageInfo = null;
console.log("Starting background script");

chrome.webNavigation.onCommitted.addListener(
	function (details) {
		if (details.frameId == 0) {
			console.log("onCommitted:", details.frameId, details.url);
			initTabInfo(details.tabId, details.url);
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
			var data = JSON.parse(event.data);
			data.method = "fill";
			if (messageRequest.method == "selectEntry") {
				var tab = tabInfo.get(messageRequest.tabId);
				console.log("Selecting from tab", tab);
				tab.pages.forEach(function(val, key, map) {
					if (val.hasLoginForm && val.hasUsernameField && val.hasPasswordField) {
						data.url = val.url;
						browser.tabs.sendMessage(messageRequest.tabId, JSON.stringify(data));
						return;
					}
				});
				tab.pages.forEach(function(val, key, map) {
					if (val.hasLoginForm) {
						data.url = val.url;
						browser.tabs.sendMessage(messageRequest.tabId, JSON.stringify(data));
						return;
					}
				});
				tab.pages.forEach(function(val, key, map) {
					if (val.hasUsernameField || val.hasPasswordField) {
						data.url = val.url;
						browser.tabs.sendMessage(messageRequest.tabId, JSON.stringify(data));
					}
				});
			}
		} else if (lastWebsocketMessage != null && lastWebsocketMessage.messageType == "pageLoaded" ) {
			console.log("Got matches", event.data, lastWebsocketMessageInfo.tabId);
			var thisTabInfo = tabInfo.get(lastWebsocketMessageInfo.tabId);
			thisTabInfo.pageMatches = JSON.parse(event.data);
			thisTabInfo.pages = new Map();
			browser.tabs.sendMessage(lastWebsocketMessageInfo.tabId, JSON.stringify({method : "loadPage"}));
			updateBrowserActionStatus();
		} else {
			console.log("Unexpected websocket message");
		}
	};

	socket.onclose = function(event) {
		console.log("WebSocket closed");
		browser.browserAction.disable();
		messageRespond = null;
		socket = null;
		updateBrowserActionStatus();
	};

	socket.onopen = function(event) {
		console.log("WebSocket opened");
		updateBrowserActionStatus();
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
		console.log("pageLoaded message recieved:", req.data, sender.tab.id, sender.frameId);
		var info = tabInfo.get(sender.tab.id);
		if (info == null) {
			initTabInfo(sender.tab.id, sender.tab.url);
			var data = {messageType: "pageLoaded", url: sender.tab.url};
			sendWebsocketMessage(data, {tabId : sender.tab.id});
		}
		tabInfo.get(sender.tab.id).pages.set(sender.frameId, req.data);
		updateBrowserActionStatus();
		return false;
	} else if (req.method == "selectEntry" || req.method == "showClient") {
		console.log(req.method, "message recieved:", req.data);
		messageRespond = res;
		messageRequest = req;
		sendWebsocketMessage(req.data);
		return true;
	} else if (req.method == "popupLoaded") {
		console.log("popupLoaded message recieved:", req.data);
		messageRespond = res;
		messageRequest = req;
		var tabLocated = function(tabA) {
			var activeTabInfo = tabInfo.get(tabA[0].id);
			var pageMatches = activeTabInfo.pageMatches;
			if (pageMatches == null) {
				//TODO: need to figure out why we get here 
				pageMatches = new Array();
			}

			var foundLoginForm = false;
			console.log("pages", activeTabInfo.pages);
			activeTabInfo.pages.forEach(function(val, key, map) {
				if (val.hasLoginForm) {
					foundLoginForm = true;
				}
			});
			messageRespond({tabId: tabA[0].id, "pageMatches": pageMatches, "hasLoginForm": foundLoginForm});
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
