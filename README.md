<p align="left">
<a href="https://github.com/Dimasick-git/Mission-Control/blob/main/LICENSE"><img alt="License" src="https://img.shields.io/badge/license-GPLv2-blue.svg"></a>
<a href="https://github.com/Dimasick-git/Mission-Control/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/Dimasick-git/Mission-Control?include_prereleases"></a>
<a href="https://github.com/Dimasick-git/Mission-Control/actions/workflows/build.yml"><img alt="Build" src="https://github.com/Dimasick-git/Mission-Control/actions/workflows/build.yml/badge.svg"></a>
<a href="https://github.com/Dimasick-git/Mission-Control/actions/workflows/sync_upstream.yml"><img alt="Upstream sync" src="https://github.com/Dimasick-git/Mission-Control/actions/workflows/sync_upstream.yml/badge.svg"></a>
</p>

# Mission-Control (Ryazhenka edition)

**EN:** Ryazhenka fork of [ndeadly/MissionControl](https://github.com/ndeadly/MissionControl) — the sysmodule that lets you pair non-Switch Bluetooth controllers (PlayStation, Xbox, Wii, 8BitDo, etc.) natively on a hacked Nintendo Switch. This fork tracks upstream daily and ships two colour-related patches: (1) it **removes** the upstream `language == Russian` SPI 0x6050 override that rewrote every controller's body/buttons colour to `#FFD700 / #0057B7` (yellow + blue) — affecting real Joy-Cons and Pro Controllers, not just emulated pads; (2) every emulated controller class now reports its real vendor RGB through virtual SPI, so DualSense shows up as white in the system menu, DualShock 4 as black, Xbox as carbon, and so on. Config path remains `/config/MissionControl/` for drop-in compatibility with the vanilla module.

---

## Что это

Это форк [ndeadly/MissionControl](https://github.com/ndeadly/MissionControl) — sysmodule под Atmosphère CFW, который позволяет подключать к Nintendo Switch чужие Bluetooth-геймпады (PS3/PS4/PS5, Xbox One, Wii/WiiU Pro, 8BitDo, Razer, PowerA, SteelSeries и десятки других) без донглов и переходников. Подробности upstream-функционала — см. оригинальный [README](https://github.com/ndeadly/MissionControl#readme).

В рамках экосистемы Ряженка ([Atmosphere](https://github.com/Dimasick-git/Atmosphere), [Ryazhahand-Overlay](https://github.com/Dimasick-git/Ryazhahand-Overlay), [RCU](https://github.com/Dimasick-git/RCU), [libryazhahand](https://github.com/Dimasick-git/libryazhahand)) этот форк используется как штатный модуль для Bluetooth-периферии.

## Главные отличия от upstream

| Что | Где живёт | Зачем |
|------|-----------|-------|
| **Удалена подмена цветов 0x6050 на жёлто-синий для русского языка** | `mc_mitm/source/controllers/switch_controller.cpp` и `emulated_switch_controller.cpp` | В ванильном модуле при `GetSystemLanguage() == 10` (русский) каждый SPI-read контроллера по адресу 0x6050 переписывался байтами `{0xff,0xd7,0x00, 0x00,0x57,0xb7, ...}` = `#FFD700 / #0057B7`. Это касалось **всех** контроллеров, в том числе настоящих Joy-Cons и Pro Controller — у русскоязычных пользователей они отображались в системе с жёлтым корпусом и синими кнопками вместо реальных заводских цветов. Блоки удалены целиком. |
| **Заводские цвета корпуса/кнопок для каждого эмулированного контроллера** | `mc_mitm/source/controllers/*.hpp` (10 классов) + `virtual_spi_flash.cpp::WriteColours()` | В ванильном модуле все эмулированные контроллеры репортят серые цвета `{0x32,0x32,0x32}`. Теперь DualSense приходит белым, DualShock 4 — чёрным, Xbox One — карбоновым и т.д. Подробнее: [`docs/RU/controllers.md`](docs/RU/controllers.md). |
| **Версия 15.1.1** (вместо 0.15.1) | `Makefile` | Поднята согласно нашей внутренней нумерации Ряженки. Совместима с тем же ams >=1.11.1 / HOS до 22.1.0, что и upstream. |
| **Makefile fallback на `v15.1.1`** | `Makefile:6` | CI shallow-checkout без тегов больше не падает с `unknown` версией. |
| **CI/CD набор** | `.github/workflows/{build,release,sync_upstream,verify_build}.yml` | Ежедневный sync с ndeadly/MissionControl (PR, не прямой merge), автосборка через `devkitpro/devkita64` Docker, авто-релиз при пуше тега или изменении Makefile. |
| **Локальный сборщик** | `scripts/build.ps1` | Windows-friendly обёртка над Docker-сборкой без необходимости ставить devkitPro/msys2 на хост. |
| **Брендинг toolbox.json** | `mc_mitm/toolbox.json` | `MissionControl (Ryazhenka)` в Tinfoil/DBI/oversight tools, чтобы отличать от ванильного. TID и путь `/config/MissionControl/` **не трогаются** — обратная совместимость с пользовательскими `missioncontrol.ini` сохранена. |

Всё остальное синхронизируется с upstream автоматически (см. `.github/sync-protected-paths.txt` — список наших файлов, которые бот защищает от перетирания).

## Установка

1. Скачайте свежий релиз: [Releases →](https://github.com/Dimasick-git/Mission-Control/releases/latest), файл `MissionControl-15.1.1-main-<hash>.zip`.
2. Распакуйте содержимое в корень SD-карты, разрешая объединение папок и перезапись существующих файлов.
3. Перезагрузите консоль. Модуль `mc.mitm` (TID `010000000000bd00`) подцепится Atmosphère при загрузке.

**Требования**: Atmosphère ≥ 1.11.1 на HOS 22.1.0 (или соответствующая связка на более старых прошивках). На системах со старым Atmosphère модуль может крашить bluetooth — обновитесь.

## Использование

Сопряжение контроллера — через штатное меню `Контроллеры → Изменить хват и порядок` (а не "Сопряжение нового контроллера", это интуитивно неправильно, но именно так задумано Nintendo). Для PS5 DualSense — зажать `PS + Share` пока подсветка не начнёт мигать сердцебиением. Для Xbox One — `guide` + кнопка sync на задней стороне. Полный список комбинаций сопряжения — в [`docs/RU/pairing.md`](docs/RU/pairing.md).

## Поддерживаемые контроллеры (с цветами после нашего фикса)

| Контроллер | Цвет корпуса | Цвет кнопок |
|------------|--------------|-------------|
| Sony DualSense / DualSense Edge | белый (Cosmic White) | тёмно-серый |
| Sony DualShock 4 (v1/v2) | чёрный (Jet Black) | белый |
| Sony DualShock 3 | угольный чёрный | серебристый |
| Microsoft Xbox One S / Elite 2 / Adaptive | карбоновый чёрный | светло-серый |
| Nintendo Wii Remote / WiiU Pro | белый | чёрный |
| 8BitDo (SN30 Pro / Zero / Ultimate 2.4G) | кремовый | тёмно-серый |
| Razer Serval / Raiju | чёрный | Razer green |
| PowerA Moga Pro / Hero | чёрный | белый |
| NVIDIA Shield (2017) | матовый чёрный | зелёный акцент |
| SteelSeries Stratus / Nimbus | матовый чёрный | светлый |
| _остальные (Ipega, Mocute, Hori, Mad-Catz и др.)_ | upstream-серый (фоллбек) | upstream-серый |

Если хочешь, чтобы твой бренд тоже получил фирменный цвет — открой [issue](https://github.com/Dimasick-git/Mission-Control/issues) с моделью и RGB.

## Конфигурация

Шаблон конфига кладётся в `/config/MissionControl/missioncontrol.ini.template`. Скопируй в `missioncontrol.ini`, раскомментируй нужные строки, перезагрузись. Все настройки upstream работают как есть — мы их не трогаем. Параметров для управления цветами из ini пока нет (если попросишь — добавлю).

## Сборка

### Локально (Windows, через Docker)

```powershell
.\scripts\build.ps1 -Dist
```

Скрипт сам подтянет образ `devkitpro/devkita64`, прокинет текущую директорию и положит результат в `dist/MissionControl-*.zip` с SHA-256.

### Локально (Linux/WSL, нативно)

```bash
pacman -Syu && pacman -S switch-libjpeg-turbo
git clone --recursive https://github.com/Dimasick-git/Mission-Control.git
cd Mission-Control && make dist -j$(nproc)
```

### В CI

`Push` в `main` → `build.yml` собирает артефакт. `git tag v15.x.x && git push --tags` → `release.yml` собирает и публикует GitHub Release.

Подробнее: [`docs/RU/build.md`](docs/RU/build.md).

## Удаление

Удалить с SD-карты:
- `/atmosphere/contents/010000000000bd00`
- `/atmosphere/exefs_patches/{bluetooth,btm,hid}_patches`
- _(опционально)_ `/config/MissionControl`

И через `Системные настройки → Контроллеры и датчики → Отключить контроллеры` сбросить базу сопряжений.

## Лицензия

GPL-2.0, наследуется от ndeadly/MissionControl. См. [`LICENSE`](LICENSE).

## Кредиты

- **ndeadly** — оригинальный MissionControl, без которого этого форка бы не было.
- **SciresM, TuxSH, hexkyz, fincs** и команда Atmosphère.
- **switchbrew** — документация Switch OS.
- **devkitPro** — toolchain.
- **Dimasick-git (Ryazhenka)** — этот форк, цветовой патч, CI/CD пайплайн.
