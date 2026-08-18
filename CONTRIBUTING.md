# Contributing — Mission-Control (Ryazhenka)

**EN:** This is a downstream fork tracking [ndeadly/MissionControl](https://github.com/ndeadly/MissionControl) daily via CI. Bugs in upstream features → file upstream. Bugs in our Ryazhenka-specific patches (per-controller colours, CI workflows, Russian docs) → file here.

---

## Что присылать сюда

- Баги в нашем цветовом патче (неправильный RGB для конкретной модели,
  крэш в `WriteColours`, цвета не применяются после релога).
- Запросы на новые controller-class overrides (например, добавить
  Xiaomi/GameSir с реальными фирменными цветами).
- Проблемы со сборкой через `scripts/build.ps1` или CI workflows.
- Опечатки/неточности в русской документации (`docs/RU/`).

## Что присылать в upstream (ndeadly/MissionControl)

- Поддержка новых контроллеров.
- Баги в сопряжении / rumble / motion.
- Изменения в exefs_patches.
- Всё что касается логики mitm.

Их issue tracker: <https://github.com/ndeadly/MissionControl/issues>.

## Как открыть PR сюда

1. Форкнуть [Dimasick-git/Mission-Control](https://github.com/Dimasick-git/Mission-Control).
2. Создать ветку от `main` (не от `master` — `master` зеркалит upstream).
3. Закоммитить изменение; коммит-стиль свободный, RU/EN — как тебе удобнее.
4. Открыть PR в `main`. На PR прогонится `verify_build.yml` (сборка без публикации).
5. Если меняешь файлы из [`.github/sync-protected-paths.txt`](.github/sync-protected-paths.txt) —
   объясни это в описании PR, чтобы daily sync-бот не оверрайдил их случайно.

## Стиль кода

C++: следуй стилю upstream (4 пробела, snake_case для переменных,
PascalCase для классов, namespace `ams::controller`). Не вводи новые
зависимости без обсуждения.

Документация: README/CHANGELOG/CONTRIBUTING — формат "1 абзац EN → `---` →
полно RU", как у остальных репо Ряженки (см. [libryazhahand](https://github.com/Dimasick-git/libryazhahand)).

## Релиз-процесс (для мейнтейнера)

Единственный источник версии — строка `APP_VERSION := X.Y.Z` в корне `Makefile`. Тег и GitHub Release всегда соответствуют ей: `vX.Y.Z`.

| Сценарий | Действие workflow |
|---|---|
| Обычный коммит, `APP_VERSION` не меняется | Собирает свежий ZIP, удаляет старые assets из релиза этой версии и загружает **только один** новый ZIP. Тег и новый релиз не создаются. |
| Коммит с изменением `APP_VERSION` | Создаёт тег `vX.Y.Z`, публикует новый GitHub Release и загружает в него один ZIP-пакет. |

Перед выпуском новой версии обновите `APP_VERSION` и соответствующий раздел `CHANGELOG.md`, затем отправьте коммит в `main`. Не создавайте тег вручную: его создаёт workflow. Для повторной сборки той же версии достаточно отправить обычный коммит в `main` или запустить workflow вручную.
