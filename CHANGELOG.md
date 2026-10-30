# Changelog
## [2.0.4]
## eSpa v2 Breaking change
If you have an eSpa v2 shipping in or before October 2026, the v2 firmware will not flash Over The Air (OTA) due to a error in the partition table these devices were shipped with. You have two options:
1. OTA update using the v2 legacy firmware
2. Remove the eSpa and reflash at flash.espa.diy

If you elect to use the v2 legacy firmware - the eSpa will continue to function with all current features. The only difference, is the firmware now contains the external dependencies. These dependencies cannot be included on the eSpa v2 modules shipped in or before October 2026 without repartitioning the flash memory (option 2 above).

### Features:
- Implement spa name changes without reboot
- Removed internet dependencies from /fota
- Back off mqtt when trying to reconnect
- Add redirect to /fota if file system is corrupt
- Add 404 Not Found response for files that don't exist
- Add sanitise button support
- Implement LittleFS and include internet dependencies on flash memory
- Enable gzip compression
- Creation of a new RemoteDebug library that includes the Web Debug pages natively 
### Dependency updates:
- Update PubSubClient to more modern and maintained libraries
### Bug fixes:
- Implement safe handling of millis()
- Improved handling of date/time in espa.js
- Json tidy ups in WebUI.cpp
- Tidy up logging in SpaInterface.cpp
- Fixes for github build process
