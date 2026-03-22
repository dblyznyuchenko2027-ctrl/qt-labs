# TypingTrainer

Застосунок для тренування сліпого друку, реалізований як Qt Widgets застосунок на C++.  
**Практична робота №6** — QMainWindow, QStackedWidget, макети, адаптивність.

## Екрани

| Екран | Опис |
|---|---|
| **Start** | Вибір уроку через QComboBox + опис уроку + кнопка Start Training |
| **Training** | Інфо-панель (Time / Speed / Accuracy) + текст для введення + віртуальна клавіатура |
| **Results** | Підсумкові значення + кнопки Restart Training / Return to Main |

## Структура UI (QStackedWidget)

```
QMainWindow
└── centralWidget
    └── QStackedWidget  (stackScreens)
        ├── pageStart    [index 0]
        ├── pageTraining [index 1]
        └── pageResults  [index 2]
```

## Ключові objectName

| objectName | Тип | Призначення |
|---|---|---|
| `stackScreens` | QStackedWidget | перемикання екранів |
| `comboLesson` | QComboBox | вибір уроку |
| `btnStartTraining` | QPushButton | перехід на Training |
| `textDisplay` | QTextEdit | відображення тексту |
| `frameKeyboard` | QFrame | контейнер клавіатури |
| `lblTimeValue` / `lblSpeedValue` / `lblAccuracyValue` | QLabel | показники сесії |
| `progressAccuracy` | QProgressBar | візуальна точність |
| `lblTimeResult` / `lblSpeedResult` / `lblAccuracyResult` | QLabel | результати |
| `btnRestartTraining` / `btnReturnToMain` | QPushButton | навігація з Results |

## Збірка

```bash
cmake -S . -B build
cmake --build build
./build/TypingTrainer
```

Вимоги: Qt 6.x, CMake ≥ 3.16, C++17.
