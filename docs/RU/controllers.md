# Поддерживаемые контроллеры и заводские цвета

Этот файл описывает per-controller цветовой патч Ryazhenka-форка
([CHANGELOG 15.1.1](../../CHANGELOG.md#1511--2026-05-26)) и связывает
имена классов C++ с RGB-значениями, которые они теперь репортят через
виртуальный SPI flash (адрес `0x6050`).

## Зачем

В ванильном `ndeadly/MissionControl` файл
[`virtual_spi_flash.cpp`](../../mc_mitm/source/controllers/virtual_spi_flash.cpp)
заполняет `0x6050` нейтральным серым:

```cpp
factory_colours = {
    {0x32, 0x32, 0x32},  // body
    {0xe6, 0xe6, 0xe6},  // buttons
    {0x46, 0x46, 0x46},  // left_grip
    {0x46, 0x46, 0x46}   // right_grip
};
```

Switch HOS получает этот серый и в меню `Контроллеры` рендерит контроллер
дефолтной палитрой — на ряде ревизий это жёлто-синяя комбинация. Для
владельцев DualSense, DualShock 4 и т.д. это выглядит как баг: цвет вообще
не соответствует реальному корпусу.

## Что делает наш патч

1. В `switch_controller.hpp` добавлены 4 виртуальных метода:

   ```cpp
   virtual RGBColour GetBodyColour()      const { return {0x32, 0x32, 0x32}; }
   virtual RGBColour GetButtonsColour()   const { return {0xe6, 0xe6, 0xe6}; }
   virtual RGBColour GetLeftGripColour()  const { return {0x46, 0x46, 0x46}; }
   virtual RGBColour GetRightGripColour() const { return {0x46, 0x46, 0x46}; }
   ```

   Дефолты — те же серые upstream-значения, чтобы classes без override
   вели себя как раньше.

2. В `virtual_spi_flash.cpp` добавлен метод:

   ```cpp
   Result VirtualSpiFlash::WriteColours(const RGBColour &body,
                                        const RGBColour &buttons,
                                        const RGBColour &left_grip,
                                        const RGBColour &right_grip);
   ```

   В отличие от `EnsureMemoryRegion`, он **безусловно** перезаписывает
   `0x6050` — то есть применяет цвета на каждый boot, даже если SPI-файл
   уже существовал с серыми дефолтами.

3. В `emulated_switch_controller.cpp::Initialize()` после
   `m_virtual_memory.Initialize(...)` вызывается:

   ```cpp
   R_TRY(m_virtual_memory.WriteColours(this->GetBodyColour(),
                                        this->GetButtonsColour(),
                                        this->GetLeftGripColour(),
                                        this->GetRightGripColour()));
   ```

   Полиморфизм подтягивает override из конкретного подкласса.

4. В 10 контроллер-классах добавлены overrides с фирменными RGB.

## Таблица RGB по классам

| Класс                       | Файл (.hpp)                       | Body                                                                        | Buttons                                                                       |
|-----------------------------|-----------------------------------|-----------------------------------------------------------------------------|-------------------------------------------------------------------------------|
| `DualsenseController`       | `dualsense_controller.hpp`        | `#ffffff` ![](https://placehold.co/12x12/ffffff/ffffff.png)                 | `#323232` ![](https://placehold.co/12x12/323232/323232.png)                   |
| `Dualshock4Controller`      | `dualshock4_controller.hpp`       | `#000000` ![](https://placehold.co/12x12/000000/000000.png)                 | `#ffffff` ![](https://placehold.co/12x12/ffffff/ffffff.png)                   |
| `Dualshock3Controller`      | `dualshock3_controller.hpp`       | `#1a1a1a` ![](https://placehold.co/12x12/1a1a1a/1a1a1a.png)                 | `#b0b0b0` ![](https://placehold.co/12x12/b0b0b0/b0b0b0.png)                   |
| `XboxOneController`         | `xbox_one_controller.hpp`         | `#141414` ![](https://placehold.co/12x12/141414/141414.png)                 | `#c8c8c8` ![](https://placehold.co/12x12/c8c8c8/c8c8c8.png)                   |
| `WiiController`             | `wii_controller.hpp`              | `#ffffff` ![](https://placehold.co/12x12/ffffff/ffffff.png)                 | `#323232` ![](https://placehold.co/12x12/323232/323232.png)                   |
| `EightBitDoController`      | `8bitdo_controller.hpp`           | `#f3e2c4` ![](https://placehold.co/12x12/f3e2c4/f3e2c4.png)                 | `#6b6b6b` ![](https://placehold.co/12x12/6b6b6b/6b6b6b.png)                   |
| `RazerController`           | `razer_controller.hpp`            | `#000000` ![](https://placehold.co/12x12/000000/000000.png)                 | `#44d62c` ![](https://placehold.co/12x12/44d62c/44d62c.png)                   |
| `PowerAController`          | `powera_controller.hpp`           | `#121212` ![](https://placehold.co/12x12/121212/121212.png)                 | `#ffffff` ![](https://placehold.co/12x12/ffffff/ffffff.png)                   |
| `NvidiaShieldController`    | `nvidia_shield_controller.hpp`    | `#1c1c1c` ![](https://placehold.co/12x12/1c1c1c/1c1c1c.png)                 | `#76b900` ![](https://placehold.co/12x12/76b900/76b900.png)                   |
| `SteelseriesController`     | `steelseries_controller.hpp`      | `#161616` ![](https://placehold.co/12x12/161616/161616.png)                 | `#b0b0b0` ![](https://placehold.co/12x12/b0b0b0/b0b0b0.png)                   |
| _Остальные классы_          | `*_controller.hpp`                | `#323232` (дефолт)                                                          | `#e6e6e6` (дефолт)                                                            |

## Как добавить новый бренд

1. Открыть `mc_mitm/source/controllers/<имя>_controller.hpp`.
2. В `public:` секцию класса (рядом с `Initialize()` или конструктором)
   добавить:

   ```cpp
   RGBColour GetBodyColour()      const override { return RGBColour{0xRR, 0xGG, 0xBB}; }
   RGBColour GetButtonsColour()   const override { return RGBColour{0xRR, 0xGG, 0xBB}; }
   RGBColour GetLeftGripColour()  const override { return RGBColour{0xRR, 0xGG, 0xBB}; }
   RGBColour GetRightGripColour() const override { return RGBColour{0xRR, 0xGG, 0xBB}; }
   ```

3. Открыть PR — `verify_build.yml` подтвердит, что собирается.

## Проверка на железе

После установки релиза на консоль с Ryazhenka Atmosphère:

1. Сопрячь любой поддерживаемый сторонний контроллер
   (`Контроллеры → Изменить хват и порядок`).
2. После успешного сопряжения зайти в `Системные настройки → Контроллеры
   и датчики → Очерёдность контроллеров`.
3. Проверить, что цвет иконки совпадает с реальным цветом корпуса
   контроллера, а не отображается жёлто-синим/серым.

Если SPI-файл контроллера уже сохранён с серыми цветами от прошлой версии
модуля — наш `WriteColours()` перезапишет его на следующем сопряжении.
Если этого не произошло, удалите вручную:
`/config/MissionControl/controllers/<MAC>/spi_flash.bin` и переподключите
контроллер.
