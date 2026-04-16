#ifndef PASSWORDLEAKCHECKER_H
#define PASSWORDLEAKCHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

/**
 * @brief Клас для безпечної перевірки пароля через Pwned Passwords API
 *
 * Використовує модель k-anonymity:
 * - Локально обчислює SHA-1 від пароля
 * - Надсилає лише перші 5 символів гешу
 * - Локально порівнює суфікс — пароль ніколи не покидає пристрій
 */
class PasswordLeakChecker : public QObject
{
    Q_OBJECT

public:
    explicit PasswordLeakChecker(QObject *parent = nullptr);

    /**
     * @brief Запустити перевірку пароля
     * @param password Пароль для перевірки (не передається в мережу)
     *
     * Рекомендується викликати після завершення введення або за кнопкою,
     * але НЕ на кожне натискання клавіші.
     */
    void checkPassword(const QString &password);

    /**
     * @brief Перевіряє, чи виконується зараз запит
     */
    bool isChecking() const;

signals:
    /**
     * @brief Перевірка завершена успішно
     * @param found true, якщо пароль знайдено у злитих базах
     * @param count кількість збігів (0, якщо не знайдено)
     */
    void checkCompleted(bool found, int count);

    /**
     * @brief Помилка під час перевірки
     * @param message Зрозуміле повідомлення про помилку
     */
    void checkFailed(const QString &message);

    /**
     * @brief Сигнал стану: почалася або завершилася перевірка
     */
    void checkingStateChanged(bool isChecking);

private slots:
    void onReplyFinished();
    void onReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkRequest buildRequest(const QByteArray &hashPrefix) const;
    bool parseResponse(const QByteArray &body, const QByteArray &suffix, int &outCount) const;

    QNetworkAccessManager m_network;
    QNetworkReply        *m_currentReply = nullptr;
    QByteArray            m_expectedSuffix;  // зберігається локально для порівняння

    static constexpr int TRANSFER_TIMEOUT_MS = 10000;
    static constexpr const char *API_BASE_URL = "https://api.pwnedpasswords.com/range/";
};

#endif // PASSWORDLEAKCHECKER_H
