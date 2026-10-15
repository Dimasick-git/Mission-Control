# Changelog

## 0.16.2

- Rebuilt 0.16.1 sources in a consistent toolchain (libstratosphere and mc_mitm against the same libnx). With the
  0.16.1 release module the console woke up 1–2 s after entering sleep in airplane mode (PMC wake source BtWakeAp)
  and repeated this all night; with the rebuilt module it sleeps normally (verified on an OLED console, HOS 23).
- No source changes in mc.mitm; bluetooth/btm/hid exefs patches unchanged.

## 0.16.1

- Updated to MissionControl 0.16.0 / upstream `71bbe2d`, including HOS 23.0.0 support.
- Removed the Russian-language replacement of official controller factory colours.
- Preserved original SPI command responses and existing button/stick processing.
- Set the build version explicitly to 0.16.1, independent of historical tags.
- Replaced outdated build/release/sync automation with a manual package build and verification workflow.
