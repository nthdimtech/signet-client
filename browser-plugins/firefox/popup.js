var sendMessage = function(id, tabId, data, response) {
	chrome.runtime.sendMessage({"path":'page-to-background', "tabId": tabId,  "method": id, "data" : data}, response);
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
		var navList = document.createElement("ol");
		for (var i = 0; i < pageMatches.length; i++) {
			var match = pageMatches[i];
			var navItem = document.createElement("li");
			var navLink = document.createElement("a");
			var fullTitle = match.path + "/" + match.title;
			navLink.href = "#";
			navLink.innerHTML = fullTitle;
			navLink.onclick = genClickHandler(tabId, match.path, match.title);
			navItem.appendChild(navLink);
			navList.appendChild(navItem);
		}
		document.body.appendChild(navList);
	});
}
