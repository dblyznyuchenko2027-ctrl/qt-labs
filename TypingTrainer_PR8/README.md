# TypingTrainer — Practical Work #8

**Theme:** File system, QDir / QFile / QFileInfo, loading lessons, QRandomGenerator.

## What's new in PR8

- **`lessons/` folder** — 6 ready `.txt` lesson files (must be placed next to the executable)
- **`scanLessons()`** — scans `lessons/*.txt` via `QDir`, fills `QComboBox` with `addItem(title, path)`
- **`loadLessonFile(path)`** — reads file via `QFile + QTextStream` (UTF-8), normalizes line endings, feeds into `TextModel`
- **Random lesson** button — `QRandomGenerator::global()->bounded(n)` picks a random index
- **Reload Lessons** button — re-scans `lessons/` without restarting the app; tries to restore previous selection
- **Edge cases handled:**
  - `lessons/` folder missing → combo is empty, Start/Random disabled, no crash
  - Folder exists but has no `.txt` files → same safe behaviour
  - File exists but can't be opened → `QMessageBox::warning()` shown, no crash
  - Random pressed with 0 lessons → guarded by `if (n <= 0) return`

## Lessons folder location

The app looks for `lessons/` in two places (in order):
1. Next to the executable: `QCoreApplication::applicationDirPath() + "/lessons"`
2. Current working directory: `QDir::currentPath() + "/lessons"`

When running from **Qt Creator**, copy or symlink `lessons/` into the build output folder,
or set the working directory in *Run Settings* to the project root.

## Build

```bash
cmake -S . -B build
cmake --build build
# Copy lessons folder next to executable:
cp -r lessons/ build/
./build/TypingTrainer
```

Requires: Qt 6.x, CMake ≥ 3.16, C++17.

## File structure

```
TypingTrainer_PR8/
├── CMakeLists.txt
├── .gitignore
├── lessons/                        ← NEW in PR8
│   ├── 01_Starter_Text.txt
│   ├── 02_Numbers_and_Symbols.txt
│   ├── 03_Common_Words_Part1.txt
│   ├── 04_Python_Code.txt
│   ├── 05_Lorem_Ipsum.txt
│   └── 06_Common_Words_Part2.txt
├── resources/resources.qrc
└── src/
    ├── main.cpp
    ├── TextModel.h / TextModel.cpp
    ├── MainWindow.h / MainWindow.cpp
    └── MainWindow.ui
```
