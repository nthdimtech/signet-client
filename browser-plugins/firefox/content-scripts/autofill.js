var usernameInput = null;
var passwordInput = null;
var submitInput = null;

var isChrome = false;

if (typeof browser === 'undefined') {
	isChrome = true;
	browser = chrome;
}

var sendMessage = function(id, data) {
	browser.runtime.sendMessage({"path":'page-to-background', "method": id, "data" : data});
}

var formTags = document.getElementsByTagName("Form");

var loginForm = null;

function getInnermostText(node)
{
	var innermost = node;
	while (innermost.children != null && innermost.children.length > 0) {
		innermost = innermost.children.item(0);
	}
	if (innermost.tagName.toLowerCase() == "img" && innermost.alt != null) {
		return  innermost.alt.trim();
	} else {
		return innermost.innerHTML.trim();
	}
}

var loginTextSet = new Set(["Sign in", "Sign In", "Signin", "Log in", "Log In", "Login", "LOGIN", "LOGON", "SIGNIN", "SIGNON", "SIGN IN", "SIGN ON", "LOG IN", "LOG ON", "Next", "NEXT"]);

for (i = 0; i < formTags.length; i++) {
	form = formTags.item(i);
	var formMethod = form.getAttribute("method"); 
	var buttons = form.getElementsByTagName("button");
	var inputs = form.getElementsByTagName("input");
	var anchors = form.getElementsByTagName("a");
	var tempSubmitInput = null;
	var tempUsernameInput = null;
	var tempPasswordInput = null;
	for (j = 0; j < buttons.length && submitInput == null; j++) {
		var button = buttons.item(j);
		if (button.getAttribute("type") == "submit" && loginTextSet.has(getInnermostText(button))) {
			tempSubmitInput = button;
		}
	}
	for (j = 0; j < inputs.length && tempSubmitInput == null; j++) {
		var input = inputs.item(j);
		if (input.getAttribute("type") == "submit") {
			var isLoginButton = false;
			if (!isLoginButton && loginTextSet.has(input.value)) {
				isLoginButton = true;
			}
			var inputParent = input.parentNode;
			if (!isLoginButton && inputParent.tagName.toLowerCase() == "span") {
				var siblings = inputParent.children;
				for (y = 0; y < siblings.length; y++) {
					var sibling = siblings.item(y);
					var siblingText = getInnermostText(sibling);
					if (loginTextSet.has(siblingText)) {
						isLoginButton = true;
						break;
					}
				}
			}
			if (isLoginButton) {
				tempSubmitInput = input;
			}
		}
	}
	for (j = 0; j < anchors.length && tempSubmitInput == null; j++) {
		var anchor = anchors.item(j);
		var anchorText = getInnermostText(anchor);
		if (loginTextSet.has(anchorText)) {
			tempSubmitInput = anchor;
			break;
		}		
	}
	for (k = 0 ; k < inputs.length && tempSubmitInput != null && (tempUsernameInput == null || tempPasswordInput == null); k++) {
		var input = inputs.item(k);
		if (input.type == "text" || input.type == "email" || input.type == null || input.type == "") {
			tempUsernameInput = input;
		} else if (input.type == "password") {
			tempPasswordInput = input;
			break;
		}
	}
	if (tempSubmitInput != null && (tempUsernameInput != null || tempPasswordInput != null))  {
		var betterMatchFound = false;
		var bestMatchFound = false;
		if (tempUsernameInput != null && tempPasswordInput != null) {
			betterMatchFound = true;
			bestMatchFound = true;
		} else if (tempUsernameInput != null && usernameInput == null && passwordInput != null) {
			betterMatchFound = true;
		} else if (tempPasswordInput != null) {
			betterMatchFound = true;
		}
		if (betterMatchFound) {
			usernameInput = tempUsernameInput;
			passwordInput = tempPasswordInput;
			submitInput = tempSubmitInput;
			if (bestMatchFound) {
				break;
			}
		}
	}
}

var data = {messageType: "pageLoaded", url: window.location.href};
if (submitInput != null && (usernameInput != null || passwordInput != null)) {
	data.hasLoginForm = true;
	data.hasUsernameField = true;
	if (passwordInput != null) {
		data.hasPasswordField = true;
	}
	sendMessage("pageLoaded", data);
}

browser.runtime.onMessage.addListener(function (req, sender, res) {
	var request = JSON.parse(req);
	if (request.method == "loadPage") {
		var data = {messageType: "pageLoaded", url: window.location.href};
		if (submitInput != null && (usernameInput != null || passwordInput != null)) {
			data.hasLoginForm = true;
			data.hasUsernameField = true;
			if (passwordInput != null) {
				data.hasPasswordField = true;
			}
			sendMessage("pageLoaded", data);
		}
	} else if (request.method == "fill") {
		if (usernameInput != null && request.username != null) {
			usernameInput.value = request.username;
		}
		if (passwordInput != null && request.password != null) {
			passwordInput.value = request.password;
		}
		if (usernameInput != null && passwordInput != null && submitInput != null) {
			submitInput.click();
		}
	}
	return true;
});
