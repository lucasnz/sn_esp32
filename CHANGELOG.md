# Changelog
## [2.0.4]
### Features:
- Implement spa name changes without reboot
- Removed internet dependencies from /fota
- Back off mqtt when trying to reconnect
- Add redirect to /fota if file system is corrupt
- Add 404 Not Found response for files that don't exist
- Add sanitise button support
- Implement LittleFS and include internet dependencies on flash memory
- Enable gzip compression
### Dependency updates:
- Update PubSubClient and RemoteDebug to more modern and maintained libraries
### Bug fixes:
- Implement safe handling of millis()
- Improved handling of date/time in espa.js
- Json tidy ups in WebUI.cpp
- Tidy up logging in SpaInterface.cpp

## [2.0.3-beta]
- Fix build for pio 6.2.0
- Update dependencies
- Fix cumulative energy when value is larger than signed int
- implement websocket on webui

## [2.0.2-beta]
- Breaking Change : MQTT base topic is now eSpa/unique_id eg eSpa/844F6033E864
- Breaking Change : File system update is required
- Feature : Add web-based debug log viewer (`/debug`) with live WebSocket console and remote reboot/level controls
- Feature : Replace RemoteDebug with WebRemoteDebug to support web and serial debug output
- Feature : MQTT connection is now established independently of the spa serial link
- Feature : Re-enable Home Assistant auto-discovery for Date Time and Day of Week
- Fix : Conversion of 2 digit year
- Fix : Correct initial `mqttLastConnect` value so MQTT reconnect backoff works from boot
- Fix : Improve reliability of web-initiated reboot

## [2.0.1-beta]
- Feature: Make `PSAV_LVL` a read-write property and implement writer (`W63:<0-2>`) to set Off/Low/High.
- Feature: Add read-write properties `PSAV_BGN` and `PSAV_END` and implement writers (`W64:<value>`, `W65:<value>`) for power-save start/end times (encoded as h*256+m).
- Fix: Simplfy Debug logging output to reduce noise.

## [2.0.0-beta]
- Rewrite SpaInterface class to be more efficient
- Add settable CLMT and sanitize cycle start time handling
- Add `ss` debug command for sending raw serial payloads to the spa controller and extended timeouts for long-running commands
- Add heat element current measurement and support W01-W07 commands in register map
- Add keyboard button press support for waking the spa
- Add CLMT & VMAX as settable properties

## [1.0.18]
- Add v2 boards
- Fix string conversion for sleep timers
   
## [1.0.17]
- Fix typo in Lock Mode autodiscovery

## [1.0.16]
- Add control panel lock mode
- Optomise code to reduce size

## [1.0.15]
- Fixed setMode 

## [1.0.14] 

### Changed
- Changed spa-base to ESP32-S3
- Disabled LED 4 on ESP32-S3 due to conflict
- Increased memory allocation
- Fix Python version rather than have it auto update to new versions
- Force 1.0.14 release

## [1.0.13] 

### Fixed
- Fixed access point address space to 169.254.1.1/24

## [1.0.12] 

### Changed
- Changed AP network to link-local address space (168.254.1.0/24)
