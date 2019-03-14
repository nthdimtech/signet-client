// Load settings with the storage API.
var gettingStoredStats = browser.storage.local.get();

const serverUrl = 'ws://localhost:910'

urlLoadOnOpen = null;

function createSocket() {
	socket = new WebSocket(serverUrl);

	socket.onmessage = function(event) {
		console.debug("WebSocket message received", event);
	};

	socket.onclose = function(event) {
		console.debug("WebSocket closed", event);
		socket = new WebSocket(serverUrl);
	};

	socket.onopen = function(event) {
		console.debug("WebSocket opened", event);
		if (urlLoadOnOpen != null) {
			console.debug("WebSocket sending URL on open", urlLoadOnOpen);
			socket.send(urlLoadOnOpen);
			urlLoadOnOpen = null;
		}
	};
}

createSocket();

socket.onerror = function(event) {
	console.debug("WebSocket error", event);
}

gettingStoredStats.then(results => {
  // Initialize the saved stats if not yet initialized.
	if (!results.stats) {
		results = {
		};
	}

	// Monitor completed navigation events and update
	browser.webNavigation.onCommitted.addListener((evt) => {
			// Filter out any sub-frame related navigation event
			if (evt.frameId !== 0) {
				return;
			}
			if (socket.readyState == WebSocket.OPEN) {
				console.debug("WebSocket sending URL", evt.url);
				socket.send(evt.url);
			} else {
				urlLoadOnOpen = evt.url;
				createSocket();
			}
	 	}, {url: [{schemes: ["http", "https"]}]});
});
