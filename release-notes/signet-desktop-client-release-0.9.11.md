Signet desktop client version 0.9.10 is a major feature release which fixes several critical bugs. The most significant features are the ability to read the contents of database backups and support for storing passwords that can later by typed by the device without the client running.

### General improvements

- The client can now read data from a database file when no device is connected. You can start reading a database file from "File->Open" and switch back to device mode by closing the file through "File->Close". Database files are read only for this release. In file mode actions that would otherwise cause the Signet device to type instead copy the data to the clipboard.

- Added the ability to store passwords into so called "Password Slots" which can be typed by the device when no client is running.  This feature is only available if your device firmware is upgraded to version 1.3.2 or higher. The slots can be programmed from "Device->Password Slots" Once programmed, to type a password slot press the device button N times to select the Nth slot. Then press and hold the device button to type.

- The client stores the window size and position on exit and restores the window to the same size and position on the next session.

- Added "--start-in-systray" command line option to start client without opening a window. This feature only works on MacOS and GNU/Linux systems

- The client now uses a generic icon for accounts with no predefined icon. This creates a more consistent look.

- Added six new predefined icons: Reddit, Amazon, Microsoft, Steam, and Tumblr

- The client now shorts entries alphabetically

### Bug fixes

- Fixed several device detection glitches on Windows and MacOS

- Fixed shutdown failures on Windows issue causing task tray icons to accumulate

- Fixed a crash bug in CSV import triggered by CSV's with variable row lengths
