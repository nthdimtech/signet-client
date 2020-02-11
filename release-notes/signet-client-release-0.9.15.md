Signet desktop client version 0.9.15 release notes

### New features

- Detect if a Signet HC is connected and disable features not yet supported by
Signet HC, use Signet HC specific file extensions, and disable features not yet
supported by Signet HC such as device emulation and cleartext passwords.

### Bug fixes

- Fix device synchronization error when backing up device. In 0.9.14 after a
  backup operation the client couldn't send any new commands to the device.

- Additional validation on database data to prevent crashes

- Don't crash when there are empty generic field names
