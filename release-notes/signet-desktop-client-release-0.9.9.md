Signet desktop client version 0.9.9 release notes

### Account Grouping

The 0.9.9 release adds support for grouping accounts and displays accounts in a tree view when at least one account is assigned to a group. When some accounts are grouped and others are not the ungrouped accounts are placed into a default "Unsorted" group. Group assignments can be made during account creation or editing by modifying the group field. Nested groups are supported by entering multiple groups separated by a '/' character similar to a file path.

Unfortunatly Group information added by the KeePass import tool in version 0.9.8.1 cannot be used by version 0.9.9. The client stored in a generic field "path" but this field was incorrectly marked as secret so GUI preventing it from being useful for display purposes. This can be fixed manually by copying the generic "path" fields of previously imported accounts into the now built-in "group" field. Alternately the KeePass import operation could be repeated, causing the imports to be grouped correctly. After the second import operation there will probably be some duplicates in the "Unsorted" group that should be removed.

### Pass Database Import

You can now import password database's created by the [pass](https://www.passwordstore.org/) command line tool. The import operation operates on one account at a time as there is no way yet to write multiple accounts at the same time

### CSV Database Import

Release 0.9.9 supports importing accounts and other data types from CSV files. When importing accounts important fields such as "username" will accept multiple similar CSV column header values (such as "user") as sources. Columns not matched to any pre-defined fields will be added as generic fields.

### General Improvements

Account passwords are now hidden by default when editing or creating an account. They can be shown by unchecking a "hide" a hide checkbox that has been added to password fields
