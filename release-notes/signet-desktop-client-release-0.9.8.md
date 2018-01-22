Signet desktop client version 0.9.8 release notes

### KeePass Import feature added

Feature allows import of KeePass 2.x databases. Import is done as a merge with
any existing data in the database, giving the option to rename, skip, or
overwrite entries if they exist on both. KeePass stores entries in a hierarchy
where Signet currently shows a flat list. To avoid losing the hierarchical data
the importer stores an additional "Path" field for every entry imported from
KeePass denotes the entries position in the hierarchy in the same way as a file
path, separated by '/' characters. A future Signet client release could use this
data to show a hierarchical view.

### General Improvements

* Changed application Icon to match the device logo and used true
  multi-resolution icons to improve rendering at low resolutions

### Keyboard Layout Detection

* On Linux/X11 systems query X11 to find out of R-Alt is used as a key modifier
  (as opposed to a character modifier) and don't generate R-Alt if it is.

* Perform scan in order of modifier not in order of scan code to prevent the
  appearance of modifier keys being "tapped" which was acting as a global hotkey
on Ubuntu's "Unity" desktop environment

### Bug fixes

* Fixed crash bug that occurred when users allowed a login operation to timeout
  before pushing the device button

* Fixed bug that caused offscreen or poorly aligned initial positioning of
  window on multi-screen displays
