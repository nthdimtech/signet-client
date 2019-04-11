var isChrome = false;

if (typeof browser === 'undefined') {
	isChrome = true;
	browser = chrome;
}

var sendMessage = function(id, tabId, data, response) {
	browser.runtime.sendMessage({"path":'page-to-background', "tabId": tabId,  "method": id, "data" : data}, response);
}

var genClickHandler = function (tabId, path, title) {
	return function() {
		sendMessage("selectEntry", tabId, {messageType: "requestFields", "path": path, "title": title, requestedFields: ["username", "password"]}, function (response) {
		});
		window.close();
	};
}

var genShowClickHandler = function (tabId, path, title) {
	return function() {
		var data = {"path": path, "title": title};
		sendMessage("showClient", tabId, {messageType: "show", "path": path, "title": title}, function (response) {
		});
		window.close();
	};
}

window.onload = function () {
	sendMessage("popupLoaded", 0, {"empty": "blah"}, function(response) {
		var pageMatches = response.pageMatches;
		var tabId = response.tabId;
		var table = document.createElement("table");
		
		if (pageMatches.length == 0) {
			var row = document.createElement("tr");
			var navLink = document.createElement("a");
			navLink.href = "#";
			navLink.innerHTML = "Show client";
			navLink.onclick = genShowClickHandler(tabId, "", "");
			row.appendChild(navLink);
			table.appendChild(row);
		} else {
			if (response.hasLoginForm) {
				var row = document.createElement("tr");
				var heading = document.createElement("td");
				heading.innerHTML = "Login";
				row.appendChild(heading);
				table.appendChild(row);
				
				row = document.createElement("tr");
				var list = document.createElement("ul");
				for (var i = 0; i < pageMatches.length; i++) {
					var match = pageMatches[i];
					var elem = document.createElement("li");
					var navLink = document.createElement("a");
					var fullTitle = match.path + "/" + match.title;
					navLink.href = "#";
					navLink.innerHTML = fullTitle;
					navLink.onclick = genClickHandler(tabId, match.path, match.title);
					elem.appendChild(navLink);
					list.appendChild(elem);
				}
				row.appendChild(list);
				table.appendChild(row);
			}
			var row = document.createElement("tr");
			var heading = document.createElement("td");
			heading.innerHTML = "Show in client";
			row.appendChild(heading);
			table.appendChild(row);
			row = document.createElement("tr");
			var list = document.createElement("ul");
			for (var i = 0; i < pageMatches.length; i++) {
				var match = pageMatches[i];
				var elem = document.createElement("li");
				var navLink = document.createElement("a");
				var fullTitle = match.path + "/" + match.title;
				navLink.href = "#";
				navLink.innerHTML = fullTitle;
				navLink.onclick = genShowClickHandler(tabId, match.path, match.title);
				elem.appendChild(navLink);
				list.appendChild(elem);
			}
			row.appendChild(list);
			table.appendChild(row);
		}

		document.body.appendChild(table);
	});
}
