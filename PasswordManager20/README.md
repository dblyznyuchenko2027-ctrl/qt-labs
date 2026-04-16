# PasswordManager — ПР19 + ПР20
## ПР19: Мережеві запити | ПР20: Асинхронність і потоки

---

## ПР20: Масова фонова перевірка паролів

### Нові файли

| Файл | Роль |
|------|------|
| `batchcheckresult.h` | Структури даних: `CredentialCheckTask`, `SingleCheckResult`, `BatchCheckResult` |
| `batchpasswordchecker.h/cpp` | Клас масової перевірки через `QtConcurrent::run()` |

### Архітектура (потоки)

```
GUI-потік                          Фоновий потік (QtConcurrent)
──────────────────────────────     ──────────────────────────────────
MainWindow::onCheckAllPasswords()
  → BatchPasswordChecker::startCheck()
  → QtConcurrent::run(lambda) ──────→ runBatchCheck()
                                         for each task:
                                           checkOnePassword()  ← синхронний HTTP
                                           invokeMethod(progressChanged) ──┐
                                           invokeMethod(singleResultReady)─┤
                                       return BatchCheckResult             │
  QFutureWatcher::finished() ←──────────────────────────────────────────  │
  onBatchFinished()                  ←─────────────────────────────────────┘
  onBatchProgressChanged()  ←── QueuedConnection (GUI-потік безпечно)
```

### Ключові правила безпеки потоків

- Фоновий код **не торкається жодного UI-об'єкта**
- Прогрес повертається через `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`
- `QFutureWatcher::finished` → `onBatchFinished` — завжди в GUI-потоці
- Скасування — через `std::atomic<bool>`, перевіряється перед кожним записом

### Підключення модулів (CMakeLists.txt)

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Concurrent Sql)

target_link_libraries(PasswordManager PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Network Qt6::Concurrent Qt6::Sql
)
```

### Сигнали BatchPasswordChecker

| Сигнал | Коли | Потік |
|--------|------|-------|
| `started(int)` | Задачу запущено | GUI |
| `progressChanged(int, int)` | Після кожного запису | GUI (QueuedConnection) |
| `singleResultReady(SingleCheckResult)` | Результат одного запису | GUI (QueuedConnection) |
| `finished(BatchCheckResult)` | Все завершено | GUI |
| `cancelled()` | Скасовано | GUI |

---

## ПР19: Перевірка одного пароля

Клас `PasswordLeakChecker` — асинхронний, через `QNetworkAccessManager`.
k-anonymity: надсилається лише перші 5 символів SHA-1, пароль не залишає пристрій.

---

## Структура файлів

```
PasswordManager/
├── CMakeLists.txt
├── batchcheckresult.h          ← структури даних (ПР20)
├── batchpasswordchecker.h/.cpp ← QtConcurrent фонова перевірка (ПР20)
├── passwordleakchecker.h/.cpp  ← мережева перевірка одного пароля (ПР19)
├── mainwindow.h/.cpp           ← UI: обидва чекери підключені
└── demo_main.cpp               ← консольний тест
```
