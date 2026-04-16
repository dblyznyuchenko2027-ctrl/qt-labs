#ifndef BATCHCHECKRESULT_H
#define BATCHCHECKRESULT_H

#include <QString>
#include <QList>

// ---------------------------------------------------------------------------
// Вхідні дані для перевірки одного запису
// ---------------------------------------------------------------------------

struct CredentialCheckTask
{
    int     id;        // ID запису в БД
    QString name;      // Назва запису (для відображення)
    QString password;  // Пароль (ніколи не передається в мережу напряму)
};

// ---------------------------------------------------------------------------
// Результат перевірки одного запису
// ---------------------------------------------------------------------------

enum class LeakStatus {
    Unknown,       // ще не перевірявся
    Clean,         // не знайдено у злитих базах
    Compromised,   // знайдено у злитих базах
    CheckFailed    // помилка мережі або сервера
};

struct SingleCheckResult
{
    int        taskId    = -1;
    LeakStatus status    = LeakStatus::Unknown;
    int        leakCount = 0;       // кількість збігів (> 0 якщо Compromised)
    QString    errorMsg;            // заповнюється при CheckFailed
};

// ---------------------------------------------------------------------------
// Агрегований результат пакетної перевірки
// ---------------------------------------------------------------------------

struct BatchCheckResult
{
    int total       = 0;
    int checked     = 0;
    int compromised = 0;
    int failed      = 0;

    QList<SingleCheckResult> results;  // по одному на кожен запис
};

#endif // BATCHCHECKRESULT_H
