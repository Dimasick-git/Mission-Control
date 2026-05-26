# Changelog — Mission-Control (Ryazhenka)

Все заметные изменения форка относительно [ndeadly/MissionControl](https://github.com/ndeadly/MissionControl)
документируются здесь. Формат — [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/).
Версионирование — SemVer-подобное, увязанное с upstream номером (upstream
`v0.15.1` → у нас `v15.1.1`).

## [15.1.1] — 2026-05-26

Первый релиз форка под брендом Ряженка.

### Исправлено (главное — то ради чего форк затевался)

- **Удалена upstream-подмена цветов SPI 0x6050 для русскоязычных
  пользователей.** В `switch_controller.cpp::HandleDataReportEvent` и
  `emulated_switch_controller.cpp::HandleHidCommandSerialFlashRead` были
  блоки, которые при `GetSystemLanguage() == 10` (SetLanguage_Russian)
  безусловно переписывали ответ контроллера на SPI-чтение по адресу
  0x6050 байтами `{0xff,0xd7,0x00, 0x00,0x57,0xb7, 0x00,0x57,0xb7,
  0x00,0x57,0xb7}` — это RGB `#FFD700 / #0057B7` (жёлтый + синий
  украинского флага). В результате **все** контроллеры (включая
  настоящие Joy-Cons и Pro Controller) у пользователей с русским языком
  системы отображались в меню `Контроллеры → Изменить хват и порядок`
  с жёлтым корпусом и синими акцентами вместо реальных заводских
  цветов. Оба блока удалены — теперь Switch получает честный SPI-ответ
  (для официальных Joy-Cons — из их физической SPI flash, для
  эмулированных — из нашего `WriteColours()`, см. ниже).

### Добавлено

- **Per-controller body colours.** Виртуальные `GetBodyColour()` /
  `GetButtonsColour()` / `GetLeftGripColour()` / `GetRightGripColour()` в
  базовом `SwitchController` + overrides в 10 major классах: DualSense,
  DualShock4, DualShock3, XboxOne, Wii(U), 8BitDo, Razer, PowerA,
  NvidiaShield, SteelSeries. Новый метод `VirtualSpiFlash::WriteColours()`
  безусловно перезаписывает регион 0x6050 при инициализации эмулированного
  контроллера. Подробнее: [`docs/RU/controllers.md`](docs/RU/controllers.md).
- **Makefile fallback** на `v15.1.1` для shallow CI-чекаутов без тегов.
- **CI/CD**: `.github/workflows/build.yml`, `release.yml`,
  `sync_upstream.yml` (ежедневный cron), `verify_build.yml`.
- **`scripts/build.ps1`** — локальная сборка через Docker под Windows.
- **`docs/RU/`** — расширенная русская документация (controllers,
  pairing, build, install).
- **`toolbox.json`**: `MissionControl (Ryazhenka)` бейдж для DBI/Tinfoil.

### Не изменилось (наследуется из upstream v0.15.1)

- TID `010000000000bd00`.
- Конфиг-путь `/config/MissionControl/` (по явной просьбе — обратная
  совместимость с пользовательскими `missioncontrol.ini`).
- mitm.lst (`btdrv`, `btm`), exefs_patches, sysmodule-логика.
- Все upstream-фичи и поддерживаемые контроллеры.

### Базовая совместимость

- Atmosphère ≥ 1.11.1.
- HOS до 22.1.0 включительно.
- Совместимо с [Dimasick-git/Atmosphere](https://github.com/Dimasick-git/Atmosphere).

[15.1.1]: https://github.com/Dimasick-git/Mission-Control/releases/tag/v15.1.1
