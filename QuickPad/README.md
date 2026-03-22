# QuickPad

Простий текстовий редактор, реалізований як Qt Widgets застосунок на C++.  
**Практична робота №5** — QAction, гарячі клавіші, Open/Save, dirty state, keyboard-first UX.

## Можливості

| Функція | Деталі |
|---|---|
| New | Ctrl+N — новий документ (з перевіркою незбережених змін) |
| Open | Ctrl+O — відкриття файлу через QFileDialog |
| Save | Ctrl+S — збереження у поточний файл або Save As якщо файл новий |
| Save As | Ctrl+Shift+S — завжди запитує шлях |
| Exit | Ctrl+Q — завершення з підтвердженням |
| Cut/Copy/Paste | Ctrl+X/C/V — стандартні команди редагування |
| Select All | Ctrl+A |
| Undo/Redo | Ctrl+Z / Ctrl+Y |
| Dirty state | Зірочка `*` у заголовку, мітка `Modified` у статусбарі |
| maybeSave | Діалог Save/Don't Save/Cancel при New, Open, Exit, закритті |
| Status bar | Повідомлення про дії + позиція курсора (Line X, Col Y) |
| Keyboard-first | Фокус повертається в редактор після будь-якої команди/діалогу |

## Збірка

```bash
cmake -S . -B build
cmake --build build
./build/QuickPad
```

Вимоги: Qt 6.x, CMake ≥ 3.16, компілятор з підтримкою C++17.

## Структура проєкту

```
QuickPad/
├── CMakeLists.txt
├── .gitignore
├── README.md
├── resources/
│   └── resources.qrc
└── src/
    ├── main.cpp
    ├── MainWindow.h
    └── MainWindow.cpp
```
