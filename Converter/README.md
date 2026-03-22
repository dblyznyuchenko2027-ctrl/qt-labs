# Converter

Практична робота №2 — Qt Widgets Application (Qt Designer + CMake)

## Опис

Застосунок-конвертер одиниць вимірювання з трьома вкладками:
- **Length** — довжина (meters, kilometers, inches, feet, miles)
- **Mass** — маса (kilograms, pounds, ounces)
- **Temperature** — температура (Celsius, Fahrenheit, Kelvin)

Інтерфейс побудований у Qt Designer (`mainwindow.ui`).  
Логіка конвертації підключається у ПР3.

## Структура objectName

| Вкладка     | Елемент         | objectName              |
|-------------|-----------------|-------------------------|
| Length      | Поле вводу (top)    | `lengthEditTop`         |
|             | Одиниця (top)       | `lengthComboTopUnit`    |
|             | Поле вводу (bottom) | `lengthEditBottom`      |
|             | Одиниця (bottom)    | `lengthComboBottomUnit` |
|             | Кнопка              | `lengthBtnConvert`      |
| Mass        | Поле вводу (top)    | `massEditTop`           |
|             | Одиниця (top)       | `massComboTopUnit`      |
|             | Поле вводу (bottom) | `massEditBottom`        |
|             | Одиниця (bottom)    | `massComboBottomUnit`   |
|             | Кнопка              | `massBtnConvert`        |
| Temperature | Поле вводу (top)    | `tempEditTop`           |
|             | Одиниця (top)       | `tempComboTopUnit`      |
|             | Поле вводу (bottom) | `tempEditBottom`        |
|             | Одиниця (bottom)    | `tempComboBottomUnit`   |
|             | Кнопка              | `tempBtnConvert`        |

## Збірка

### Вимоги
- Qt 6 (з модулем Widgets)
- CMake ≥ 3.16
- Компілятор з підтримкою C++17

### Qt Creator
`File → Open File or Project → CMakeLists.txt` → `Build → Run`

### Командний рядок
```bash
cmake -S . -B build
cmake --build build
./build/Converter
```

## Структура проєкту

```
Converter/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
├── .gitignore
└── README.md
```
