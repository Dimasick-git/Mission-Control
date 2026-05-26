# Сборка

Есть три пути сборки — выбери по вкусу:

| Метод               | Кому | Что нужно установить |
|---------------------|------|----------------------|
| `scripts/build.ps1` | Windows, не хочешь возиться с devkitPro | Docker Desktop |
| Нативно msys2       | Windows, частая разработка | msys2 + devkitPro pacman |
| Нативно Linux/WSL   | Linux/WSL | devkitPro pacman |
| CI (GitHub Actions) | Хочешь публиковать релизы | ничего, всё на стороне GitHub |

## Метод 1. Локально на Windows через Docker

```powershell
git clone --recursive https://github.com/Dimasick-git/Mission-Control.git
cd Mission-Control
.\scripts\build.ps1 -Dist
```

- `-Dist` — собрать и упаковать в `dist/MissionControl-*.zip`.
- `-Clean` — `make clean` перед сборкой.
- `-Tag v15.1.2` — поставить локальный git-тег перед сборкой
  (для перебивки версии без коммита).

Скрипт:
1. Проверит, что Docker запущен (`docker version`).
2. Дёрнет образ `devkitpro/devkita64:latest`.
3. Прокинет текущую директорию как `/project` и выполнит `make` внутри.
4. Положит результат в `dist/`, выведет SHA-256 для верификации.

Если Docker не установлен — скрипт напечатает инструкции по
установке (см. ниже метод 2).

## Метод 2. Нативно (msys2, Windows)

1. Поставить [msys2](https://www.msys2.org/), запустить **MSYS2 MinGW UCRT64** shell.
2. Добавить devkitPro pacman:

   ```bash
   pacman-key --recv BC26F752D25B92CE272E0F44F7FD5492264BB9D0 --keyserver keyserver.ubuntu.com
   pacman-key --lsign BC26F752D25B92CE272E0F44F7FD5492264BB9D0
   pacman -U https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.xz
   ```

   Добавить в `/etc/pacman.conf`:

   ```
   [dkp-libs]
   Server = https://pkg.devkitpro.org/packages
   [dkp-windows]
   Server = https://pkg.devkitpro.org/packages/windows/$arch/
   ```

3. Установить toolchain и зависимости:

   ```bash
   pacman -Syu
   pacman -S switch-dev switch-libjpeg-turbo
   ```

4. Прописать env (постоянно — через `~/.bashrc`):

   ```bash
   export DEVKITPRO=/opt/devkitpro
   export DEVKITA64=/opt/devkitpro/devkitA64
   export PATH=$DEVKITPRO/tools/bin:$PATH
   ```

5. Собирать:

   ```bash
   git clone --recursive https://github.com/Dimasick-git/Mission-Control.git
   cd Mission-Control
   make dist -j$(nproc)
   ```

## Метод 3. Linux / WSL2

Аналогично msys2, но проще:

```bash
# Установить ключ и репо devkitPro (см. https://devkitpro.org/wiki/devkitPro_pacman)
sudo dkp-pacman -Syu
sudo dkp-pacman -S switch-dev switch-libjpeg-turbo

git clone --recursive https://github.com/Dimasick-git/Mission-Control.git
cd Mission-Control
export DEVKITPRO=/opt/devkitpro
make dist -j$(nproc)
```

## Метод 4. CI (GitHub Actions)

- **Любой push в `main` или PR** → [`build.yml`](../../.github/workflows/build.yml)
  собирает и аплоадит артефакт (можно скачать из run-страницы).
- **Push тега `v*.*.*`** или **изменение `Makefile` в `main`** →
  [`release.yml`](../../.github/workflows/release.yml) собирает и
  публикует GitHub Release с прикреплённым zip.
- **Каждый день в 03:00 UTC** → [`sync_upstream.yml`](../../.github/workflows/sync_upstream.yml)
  тянет изменения из `ndeadly/MissionControl` и открывает PR.
- **PR** → [`verify_build.yml`](../../.github/workflows/verify_build.yml)
  smoke-тест, только сборка без публикации.

## Что внутри собранного `.zip`

```
atmosphere/
├── contents/010000000000bd00/
│   ├── exefs.nsp              ← собственно sysmodule
│   ├── mitm.lst               ← btdrv, btm
│   ├── toolbox.json
│   └── flags/boot2.flag
└── exefs_patches/             ← патчи bluetooth, btm, hid
config/MissionControl/
└── missioncontrol.ini.template
```

Распакуй содержимое в корень SD-карты с перезаписью — установка готова.

## Проблемы

- **`devkitpro/devkita64: not found`** в Docker — проверь интернет и
  `docker pull devkitpro/devkita64:latest`.
- **`switch-libjpeg-turbo missing`** — `(dkp-)pacman -S switch-libjpeg-turbo`.
- **`unknown` в названии zip** — нет git-тегов. Создай тег
  `git tag v15.1.1` перед сборкой или прими fallback из Makefile.
- **Долгая сборка libstratosphere первый раз** (5-15 минут) — это норма.
  Дальнейшие сборки кешируются.
