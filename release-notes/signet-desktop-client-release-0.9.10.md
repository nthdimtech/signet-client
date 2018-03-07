Signet desktop client version 0.9.10 is a bug fix and usability improvement release

### General improvements

- Entry search no longer only searches the start of an entry name. A search query will now match all entries that contain a word that starts with or ends with the filter text. For the purposes of search a word is any block of text that follows a whitespace, a symbol, or a change in letter case. This change should help make finding entries in larger databases much easier.

- CSV import will populate the built-in account "group" field. This enables CSVs with grouping information to be imported correctly

- You can minimize the Signet window with CTRL+W and quit with CTRL+Q

- When one or more entries cannot be parsed, the client reports a warning to the user. Parsing errors can occur when a client version introduces a new version of a data type and an older client attempts and fails to read the entries due to the version mismatch.

- Most menu actions now have keyboard accelerators.

- The client now zero pads the month and day portions of backup file names

### Bug fixes

- The KeePass importer no longer creates an incorrect group heirarchy when a group contains two sibling groups.

- The client no longer creates incorrect heirarchies when entries or groups contain "/" in their names.

- Incomplete backups files are removed by the client when you cancel a backup operation.

