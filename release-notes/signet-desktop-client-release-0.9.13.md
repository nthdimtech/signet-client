Signet desktop client version 0.9.13 release notes

### New features

- The client is integrated with a browser plugin supporting Firefox and Chrome. The client will listen for a connection from the plugin and allow it to recieve non-secret meta-data and request
secret data for individual entries. The initial version uses this functionality to login to websites without the need for Signet to act as a USB keyboard.

- Custom fields can be created with type "Text (Secret)" to indicate that their contents should not be shown unless a "show" checkbox is unchecked. Secret fields function essentially the same as password fields.

### Improvements

- Make the "group" property selectable from an combo box to make it easy to place an entry into an already created group

- When selecting a backup to load the most recent one will be automatically selected

- Always minimize the Window after initiating a copy or type operation. This is usually convienent since it allows you to see the window you want to copy or type the data into. It is also ensures that the window can be brought back to the foreground later by pressing the Signet button. When the window is already maximized then some operating systems will not allow the Signet button press
handler to raise the window to the foreground. Instead the window will flash requiring the user to foreground it.

### Bug fixes

- Add missing keyboard scancode to the keyboard detection code. The missing scancode was present on many non-US keyboards and without it some non-US keyboard layouts were not being detected
completely

- Make generic fields revert correctly when "undo" is pressed when editing an entry

- The email field in accounts now autofills to the same value as the username when the username is an email address. This behavior was present in earlier versions of the client but was lost for several versions.

- Fix various bugs occuring when multiple modal dialogs are visible at once
