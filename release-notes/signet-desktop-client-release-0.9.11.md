Signet desktop client version 0.9.11 is a major feature release that also fixes several critical bugs.

The most significant features are the ability to read the contents of database backup files, and support for typing specific stored passwords without the client running (such as during initial login).

### New features

- The client can now read data from a database file when no device is connected. You can start reading a database file from "File-&rsaquo;Open" and switch back to device mode by closing the file through "File-&rsaquo;Close". Database files are read only for this release. In file mode, actions that would otherwise cause the Signet device to type data instead copy data to the clipboard.

- The device can now type passwords stored in a "Password Slot" without a running client. This feature is only available if the device firmware version is 1.3.2 or higher. You can program the slots from **Device->Password Slots**. Once programmed, you can type a password slot by pressing the device button N times to select the Nth slot, then press and hold the device button to type the password.

### General Improvements

- The client stores the window size and position upon exit and restores the window to the same size and position on the next session.

- Added a *--start-in-systray* command line option to start the client without opening a window. This feature only works on MacOS and GNU/Linux systems.

- The client now uses a generic icon for accounts with no predefined icon. This creates a more consistent look.

- Added six new predefined icons: Reddit, Amazon, Microsoft, Steam, and Tumblr.

- The client now sorts entries alphabetically.

### Bug fixes

- Fixed several device detection glitches on Windows and MacOS.

- Fixed shutdown failures on Windows causing task tray icons to accumulate.

- Fixed a crash bug in CSV import triggered by CSVs with variable row lengths.
