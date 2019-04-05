var isChrome = false;

if (isChrome) {
	browser = chrome;
}

var sendMessage = function(id, tabId, data, response) {
	browser.runtime.sendMessage({"path":'page-to-background', "tabId": tabId,  "method": id, "data" : data}, response);
}

var genClickHandler = function (tabId, path, title) {
	return function() {
		console.log("Match selected:", path, title);
		var data = {"path": path, "title": title};
		sendMessage("selectEntry", tabId, {messageType: "requestFields", "path": path, "title": title, requestedFields: ["username", "password"]}, function (response) {
			console.log("selectEntry response:", response);	
		});
		window.close();
	};
}

window.onload = function () {
	sendMessage("popupLoaded", 0, {"empty": "blah"}, function(response) {
		console.log("Popup got message", response);
		var pageMatches = response.pageMatches;
		var tabId = response.tabId;
		var table = document.createElement("table");
		for (var i = 0; i < pageMatches.length; i++) {
			var match = pageMatches[i];
			var row = document.createElement("tr");
			var navLink = document.createElement("a");
			var fullTitle = match.path + "/" + match.title;
			navLink.href = "#";
			navLink.innerHTML = fullTitle;
			navLink.onclick = genClickHandler(tabId, match.path, match.title);
			row.appendChild(navLink);
			table.appendChild(row);
		}
		document.body.appendChild(table);
	});
}
