# CatalogFilms

Локальний desktop-застосунок для ведення особистого каталогу фільмів. Дозволяє додавати, переглядати, редагувати, видаляти та шукати фільми. Працює офлайн — дані зберігаються у SQLite-базі.

## Стек

- **Мова:** C++17
- **GUI:** Qt 6 Widgets
- **БД:** SQLite (через `Qt6::Sql`)
- **Збірка:** CMake ≥ 3.16

## Архітектура

```
CatalogFilms/
├── CMakeLists.txt
├── .gitignore
├── README.md
│
├── ui/                  # Qt Designer форми
│   ├── MainWindow.ui
│   ├── MovieDialog.ui
│   └── SettingsWindow.ui
│
├── src/                 # Реалізація
│   ├── main.cpp
│   ├── MainWindow.cpp
│   ├── MovieDialog.cpp
│   ├── SettingsWindow.cpp
│   ├── DatabaseManager.cpp
│   ├── MovieRepository.cpp
│   ├── SettingsRepository.cpp
│   ├── MovieService.cpp
│   ├── ValidationService.cpp
│   ├── SettingsManager.cpp
│   ├── SearchManager.cpp
│   ├── MovieTableModel.cpp
│   └── MovieTableWidget.cpp
│
├── include/             # Заголовкові файли
│   ├── Movie.h
│   ├── Settings.h
│   ├── MainWindow.h
│   ├── MovieDialog.h
│   ├── SettingsWindow.h
│   ├── DatabaseManager.h
│   ├── MovieRepository.h
│   ├── SettingsRepository.h
│   ├── MovieService.h
│   ├── ValidationService.h
│   ├── SettingsManager.h
│   ├── SearchManager.h
│   ├── MovieTableModel.h
│   └── MovieTableWidget.h
│
└── database/            # Місце для локальної БД (movies.db створюється у AppDataLocation)
```

Розділена на три шари:

- **UI-шар:** `MainWindow`, `MovieDialog`, `SettingsWindow`, `MovieTableWidget`.
- **Шар логіки:** `MovieService`, `ValidationService`, `SearchManager`, `SettingsManager`.
- **Шар даних:** `DatabaseManager`, `MovieRepository`, `SettingsRepository`.

## Можливості (MVP)

- Перегляд списку фільмів у таблиці.
- Додавання, редагування, видалення фільму.
- Пошук за назвою (без урахування регістру).
- Фільтрація за статусом (Усі / Переглянуто / Хочу подивитися).
- Сортування за стовпцями.
- Налаштування: тема (світла/темна), стовпець та порядок сортування, кількість фільмів на сторінці.
- Збереження стану між запусками у `movies.db`.

## Поля фільму

Обов'язкові: `id`, назва, жанр, рік, статус.
Опційні: оцінка (0-10), коментар.

## Збірка та запуск

### Linux (Ubuntu)

```bash
sudo apt-get install -y cmake qt6-base-dev qt6-tools-dev qt6-tools-dev-tools libqt6sql6-sqlite

cmake -S CatalogFilms -B CatalogFilms/build
cmake --build CatalogFilms/build -j

./CatalogFilms/build/CatalogFilms
```

### Windows (Qt Creator)

1. Встановити Qt 6.x із компонентами **Qt 6 Widgets** та **Qt 6 SQL**.
2. Відкрити `CatalogFilms/CMakeLists.txt` у Qt Creator.
3. Натиснути **Run**.

## Розташування БД

Базу даних створюється у каталозі, який повертає `QStandardPaths::AppDataLocation`:

- **Linux:** `~/.local/share/QtPractice/CatalogFilms/movies.db`
- **Windows:** `%APPDATA%\QtPractice\CatalogFilms\movies.db`
- **macOS:** `~/Library/Application Support/QtPractice/CatalogFilms/movies.db`
