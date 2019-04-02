var usernameInput = null;
var passwordInput = null;
var submitInput = null;

var handleResponse = function(message) {
	console.log("Content script got message:", message);
	/*
	var msg = JSON.parse(message);
	if (msg.length == 1) {

		if (usernameInput != null && msg[0].username != null) {
			usernameInput.value = msg[0].username;
		}
	}*/
}

var handleError = function(error) {
	console.log("Content script got error:", error);
}

var sendMessage = function(id, data) {
	var sending = chrome.runtime.sendMessage({"path":'page-to-background', "method": id, "data" : data});
	sending.then(handleResponse, handleError);
}

var formTags = document.getElementsByTagName("Form");

var loginForm = null;

console.debug("Found", formTags.length, "forms");

function getInnermostText(node)
{
	var innermost = button;
	while (innermost.children != null && innermost.children.length > 0) {
		innermost = innermost.children.item(0);
	}
	return innermost.innerHTML.trim();
}

for (i = 0; i < formTags.length; i++) {
	form = formTags.item(i);
	var formMethod = form.getAttribute("method"); 
	var buttons = form.getElementsByTagName("button");
	var inputs = form.getElementsByTagName("input");
	var loginTextSet = new Set(["Sign in", "Sign In", "Signin", "Log in", "Log In", "Login", "LOGIN", "LOGON", "SIGNIN", "SIGNON", "SIGN IN", "SIGN ON", "LOG IN", "LOG ON"]);
	for (j = 0; j < buttons.length && submitInput == null; j++) {
		var button = buttons.item(j);
		if (button.getAttribute("type") == "submit" && loginTextSet.has(getInnermostText(button))) {
			submitInput = button;
			console.log("Login button found on form", form.id, "with text", button.innerHTML.trim());
		}
	}
	for (j = 0; j < inputs.length && submitInput == null; j++) {
		var input = inputs.item(j);
		if (input.getAttribute("type") == "submit") {
			var isLoginButton = false;
			if (!isLoginButton && loginTextSet.has(input.value)) {
				console.log("Login input found on form", form.id);
				isLoginButton = true;
			}
			var inputParent = input.parentNode;
			if (!isLoginButton && inputParent.tagName.toLowerCase() == "span") {
				var siblings = inputParent.children;
				for (y = 0; y < siblings.length; y++) {
					var sibling = siblings.item(y);
					if (loginTextSet.has(sibling.innerHTML.trim())) {
						console.log("Spanned login input found on form", form.id, "with text", sibling.innerHTML.trim());
						isLoginButton = true;
						break;
					}
				}
			}
			if (isLoginButton) {
				submitInput = input;
			}
		}
	}
	for (k = 0 ; k < inputs.length && submitInput != null && (usernameInput == null || passwordInput == null); k++) {
		var input = inputs.item(k);
		if (usernameInput == null) {
			if (input.type == "text" || input.type == "email" || input.type == null || input.type == "") {
				console.log("Setting input", input.id, "as username");
				usernameInput = input;
			}
		} else if (passwordInput == null) {
			if (input.type == "password") {
				console.log("Setting input", input.id, "as password");
				passwordInput = input;
			}
		}
	}
	if (submitInput != null && usernameInput != null) {
		break;
	}
}

var data = {messageType: "pageLoaded", url: window.location.href};
if (submitInput != null && usernameInput != null) {
	data.hasLoginForm = true;
	data.hasUsernameField = true;
	if (passwordInput != null) {
		data.hasPasswordField = true;
	}
	sendMessage("pageLoaded", data);
} else {
	data.hasLoginForm = true;
	sendMessage("pageLoaded", data);
}

browser.runtime.onMessage.addListener(function (req, sender, res) {
	var request = JSON.parse(req);
	console.log("Page message from the background script:", request);
	if (usernameInput != null && request.username != null) {
		console.log("Setting username");
		usernameInput.value = request.username;
	}
	if (passwordInput != null && request.password != null) {
		console.log("Setting password");
		passwordInput.value = request.password;
	}
	if (submitInput != null) {
		submitInput.click();
	}
	return true;
});
