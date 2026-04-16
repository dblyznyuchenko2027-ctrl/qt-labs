# PasswordManager — Практична робота №19
## Мережеві запити у Qt Widgets. QNetworkAccessManager.

---

## Що реалізовано у ПР19

### Новий клас: `PasswordLeakChecker`

Окремий клас мережевого шару, що відповідає **виключно** за:
- обчислення SHA-1 гешу пароля локально (`QCryptographicHash`)
- формування запиту до [Pwned Passwords API](https://haveibeenpwned.com/API/v3#PwnedPasswords)
- асинхронну обробку відповіді (`QNetworkReply::finished`)
- розбір текстового формату `SUFFIX:COUNT`
- повернення результату через сигнали

### Принцип k-anonymity (безпечна перевірка)

```
Пароль → SHA-1 → "5A4DA..."
                   ^^^^^   ← надсилається в мережу (тільки перші 5 символів)
                        ^^ ← порівнюється локально (ніколи не покидає пристрій)
```

### Задіяні класи Qt (ПР19)

| Клас | Роль |
|------|------|
| `QNetworkAccessManager` | Центр мережевої взаємодії, надсилає GET-запит |
| `QNetworkRequest` | Описує URL, заголовки, таймаут |
| `QNetworkReply` | Відповідь сервера; читається асинхронно у `finished()` |
| `QCryptographicHash` | SHA-1 від пароля — локально |
| `QJsonDocument` | Готовий до використання для JSON API |

### Підключення модуля у CMakeLists.txt

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

target_link_libraries(PasswordManager PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Network Qt6::Sql
)
```

---

## Сигнали PasswordLeakChecker

```cpp
// Перевірка успішна
void checkCompleted(bool found, int count);

// Мережева або серверна помилка
void checkFailed(const QString &message);

// Індикатор стану (показати/сховати spinner)
void checkingStateChanged(bool isChecking);
```

---

## Коли запускати перевірку

| Сценарій | Рекомендація |
|----------|-------------|
| Після кожного символу | ❌ Не рекомендується (приватність) |
| Кнопка "Check Password" | ✅ Оптимально |
| Після втрати фокусу полем | ✅ Прийнятно |
| Під час збереження запису | ✅ Прийнятно |

---

## Обробка помилок

- `QNetworkReply::errorOccurred` → `checkFailed(message)`
- HTTP-статус ≠ 200 → `checkFailed(message)`
- Таймаут (`setTransferTimeout(10000)`) → `OperationCanceledError` → `checkFailed`
- Помилки TLS/SSL → окреме повідомлення

---

## Структура файлів (ПР19)

```
PasswordManager/
├── CMakeLists.txt              ← Qt::Network підключено
├── passwordleakchecker.h       ← новий клас (ПР19)
├── passwordleakchecker.cpp     ← реалізація мережевої логіки
├── mainwindow.h                ← оновлено: додано m_leakChecker
├── mainwindow.cpp              ← onCheckPasswordLeaked, onLeakCheck*
└── demo_main.cpp               ← консольний тест без UI
```

---

## Запуск консольного демо

```bash
# Збірка
qt-cmake -S . -B build && cmake --build build

# Тест з паролем за замовчуванням (password123 — відомий витік)
./build/PasswordManager

# Тест з власним паролем
./build/PasswordManager "MySecurePassword2024!"
```
