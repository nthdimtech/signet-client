Signet desktop client version 0.9.14 release notes

### New features

- Miscellaneous and generic entries can now be categorized into groups.

- Display device button wait prompt in the main window instead of a popup
dialog. This fixes some inconsistencies in application behavior on different
platforms.

### Improvements

- Only background application for the keyboard based "login" action

- Leave current entry selected after executing a command. The deselection
behavior was meant to make it easier to search for a new entry but it has been
confusing to too many users.   

- Sort entries alphabetically instead of by creation order

- Set some icon and button sizes in style sheet to make application appearance
more consistent

- Allow browser plugin support to be disabled with a setting

- Allow browser plugin support to be disabled at build time

### Bug fixes

- Truncate long strings in entry fields to prevent crashes

- Fix broken download link used in the application update check and about dialog

- Accurately display update check interval in update check dialog
