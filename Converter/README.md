# Converter

Практична робота №3 — Qt Widgets, сигнали/слоти, валідація, автоперерахунок.

## Функціонал

- **3 вкладки**: Length, Mass, Temperature
- **Автоматичний перерахунок** при редагуванні будь-якого поля (без кнопок)
- **Двонаправлений**: зміна верхнього → оновлює нижнє, і навпаки
- **Зміна одиниці** в ComboBox → миттєвий перерахунок
- **Перемикання вкладки** → перерахунок з поточними значеннями
- **Валідація** через `QDoubleValidator` + перевірка `toDouble(&ok)`; некоректне поле підсвічується червоним
- **Без циклічних оновлень** — використано `textEdited` + `QSignalBlocker`

## Архітектура

| Файл | Відповідальність |
|---|---|
| `converter.h/cpp` | Всі формули і коефіцієнти, без UI |
| `mainwindow.ui` | Структура інтерфейсу (Qt Designer) |
| `mainwindow.h/cpp` | Сигнали/слоти, зчитування UI, виклик Converter |
| `main.cpp` | Точка входу |

### Базові одиниці
- Length → **meter**
- Mass → **kilogram**
- Temperature → **kelvin**

## Збірка

```bash
cmake -S . -B build
cmake --build build
./build/Converter
```

Або через Qt Creator: `File → Open → CMakeLists.txt` → `Run`.
