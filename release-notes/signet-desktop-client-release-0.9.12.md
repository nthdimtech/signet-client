Signet desktop client version 0.9.12.1 release notes

### New features

- You can now create new custom data types in the client. To create a new data type select the "data types" option in the main window pulldown, add a new entry, and configure the fields that you want the data type to have. Once created an entry on the main window pulldown will be available for the data type. From there you can create entries of that data type. 

### Improvements

- Database import operations require only a single long press. (requires firmware version 1.3.4+)

### Bug fixes

- Fixed issue introduced in 0.9.12.0 causing newly created counts to not immediate appear in list

- Slowed down typing rate to 30ms per character. The maximum rate caused some systems for registering incorrect characters. (requires firemware version 1.3.4+)

- Fixed issue resulting in incorrect characters being typed when sequential characters had the same physical key (i.e. 'a' and 'A')

- Fixed issue causing crashes when importing data

- Prevented client from requesting to perform a backup when device is disconnected

- Fixed several glitches causing unexpected or incorreect behavior on entry selection
