# Changelog

## 0.16.2

- Fixed the console waking up 1–2 s after entering sleep in airplane mode (PMC wake source BtWakeAp, bit 20 of
  WAKE2_STATUS) and repeating this all night with the 0.16.1 module. On an OLED console with HOS 23: without
  Mission Control, or with only its exefs patches, sleep is clean; with the 0.16.1 process it loops; with this build
  it sleeps and wakes only on the power button.
- btdrv `GetEventInfo`/`GetHidEventInfo`/`GetBleManagedEventInfo` copied the size requested by the client out of a
  fixed event struct, past it into whatever globals followed. They now copy at most our buffer and zero the rest.
- Sleep trace: power state requests, bluetooth init/enable, how long each forwarded event waited for its reader and
  the buffer size each client asks for, in `sdmc:/config/MissionControl/sleep_trace.log` (restarts at 256 KB).
- A rebuild of 0.16.1 without these changes produces byte-identical code and still loops, so the release does not
  rely on the toolchain.

## 0.16.1

- Updated to MissionControl 0.16.0 / upstream `71bbe2d`, including HOS 23.0.0 support.
- Removed the Russian-language replacement of official controller factory colours.
- Preserved original SPI command responses and existing button/stick processing.
- Set the build version explicitly to 0.16.1, independent of historical tags.
- Replaced outdated build/release/sync automation with a manual package build and verification workflow.
