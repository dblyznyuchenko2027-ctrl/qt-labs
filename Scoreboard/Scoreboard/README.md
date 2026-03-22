# Scoreboard

Практична робота №1 — Qt Widgets Application

## Опис

Застосунок-табло для двох гравців (Player A / Player B).  
Дозволяє збільшувати та зменшувати рахунок кожного гравця, а також скидати обидва рахунки.

## Функціонал

- Відображення рахунку для Player A і Player B
- Кнопки `+` і `−` для кожного гравця
- Кнопка `Reset` для скидання обох рахунків до нуля

## Збірка

### Вимоги

- Qt 6 (з модулем Widgets)
- CMake ≥ 3.16
- Компілятор з підтримкою C++17 (MSVC / MinGW / GCC / Clang)

### Кроки

```bash
# Клонувати репозиторій
git clone <url>
cd Scoreboard

# Зібрати через Qt Creator:
# File → Open File or Project → CMakeLists.txt
# Натиснути Build → Run
```

Або через командний рядок:

```bash
cmake -S . -B build
cmake --build build
./build/Scoreboard
```

## Структура проєкту

```
Scoreboard/
├── CMakeLists.txt
├── main.cpp
├── .gitignore
└── README.md
```
