#include "passwordleakchecker.h"

#include <QCryptographicHash>
#include <QNetworkRequest>
#include <QUrl>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

PasswordLeakChecker::PasswordLeakChecker(QObject *parent)
    : QObject{parent}
{}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void PasswordLeakChecker::checkPassword(const QString &password)
{
    // Не запускати паралельний запит, якщо попередній ще не завершений
    if (m_currentReply && !m_currentReply->isFinished()) {
        m_currentReply->abort();
    }

    // 1. Локально обчислюємо SHA-1 від пароля (UTF-8)
    const QByteArray sha1 = QCryptographicHash::hash(
                                password.toUtf8(),
                                QCryptographicHash::Sha1
                            ).toHex().toUpper();

    // 2. Розбиваємо: перші 5 символів — надсилаємо, решта — порівнюємо локально
    const QByteArray prefix = sha1.left(5);
    m_expectedSuffix = sha1.mid(5);  // зберігаємо для подальшого порівняння

    // 3. Формуємо і надсилаємо запит
    const QNetworkRequest request = buildRequest(prefix);
    m_currentReply = m_network.get(request);

    // 4. Підключаємо сигнали — асинхронна обробка без блокування UI
    connect(m_currentReply, &QNetworkReply::finished,
            this, &PasswordLeakChecker::onReplyFinished);

    connect(m_currentReply, &QNetworkReply::errorOccurred,
            this, &PasswordLeakChecker::onReplyError);

    emit checkingStateChanged(true);
}

bool PasswordLeakChecker::isChecking() const
{
    return m_currentReply && !m_currentReply->isFinished();
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void PasswordLeakChecker::onReplyFinished()
{
    // Qt рекомендує не видаляти reply прямо у слоті — використовуємо deleteLater()
    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;
    emit checkingStateChanged(false);

    // Якщо була мережева помилка — вже оброблена в onReplyError, просто виходимо
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    // Перевіряємо HTTP-статус
    const int httpStatus = reply->attribute(
                               QNetworkRequest::HttpStatusCodeAttribute
                           ).toInt();

    if (httpStatus != 200) {
        emit checkFailed(
            tr("Сервер повернув статус %1. Спробуйте пізніше.").arg(httpStatus)
        );
        reply->deleteLater();
        return;
    }

    // Зчитуємо тіло відповіді — plain text формат: SUFFIX:COUNT\r\n...
    const QByteArray body = reply->readAll();
    reply->deleteLater();

    int count = 0;
    const bool found = parseResponse(body, m_expectedSuffix, count);

    emit checkCompleted(found, count);
}

void PasswordLeakChecker::onReplyError(QNetworkReply::NetworkError error)
{
    // finished() прийде після errorOccurred(), тому тут лише формуємо повідомлення
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    QString message;
    switch (error) {
    case QNetworkReply::OperationCanceledError:
        // Таймаут або abort() — формуємо зрозуміле повідомлення
        message = tr("Час очікування відповіді від сервера вичерпано. "
                     "Перевірте підключення до інтернету.");
        break;
    case QNetworkReply::HostNotFoundError:
        message = tr("Неможливо з'єднатися з сервером перевірки паролів. "
                     "Перевірте підключення до інтернету.");
        break;
    case QNetworkReply::SslHandshakeFailedError:
        message = tr("Помилка SSL/TLS з'єднання. "
                     "Перевірку пароля виконано не було.");
        break;
    default:
        message = tr("Помилка мережі: %1").arg(reply->errorString());
        break;
    }

    emit checkFailed(message);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

QNetworkRequest PasswordLeakChecker::buildRequest(const QByteArray &hashPrefix) const
{
    const QString url = QString::fromLatin1(API_BASE_URL) + QString::fromLatin1(hashPrefix);

    QNetworkRequest request{QUrl{url}};

    // Ідентифікація клієнта — рекомендована документацією HIBP
    request.setRawHeader("User-Agent", "PasswordManagerCourse/1.0");

    // Padding: відповідь доповнюється фіктивними рядками з count=0,
    // щоб за розміром відповіді не можна було визначити конкретний префікс
    request.setRawHeader("Add-Padding", "true");

    // Таймаут передавання (Qt 6): перерве запит, якщо 10 с не було обміну байтами
    request.setTransferTimeout(TRANSFER_TIMEOUT_MS);

    return request;
}

bool PasswordLeakChecker::parseResponse(const QByteArray &body,
                                        const QByteArray &suffix,
                                        int &outCount) const
{
    // Формат відповіді — рядки виду: SUFFIX:COUNT
    // Суфікс у відповіді завжди у верхньому регістрі
    const QByteArray upperSuffix = suffix.toUpper();

    const QList<QByteArray> lines = body.split('\n');
    for (const QByteArray &line : lines) {
        // Прибираємо можливий \r (відповідь може містити CRLF)
        const QByteArray trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;

        const QList<QByteArray> parts = trimmed.split(':');
        if (parts.size() < 2) continue;

        const QByteArray apiSuffix = parts.value(0);
        const int count = parts.value(1).toInt();

        if (apiSuffix == upperSuffix) {
            outCount = count;
            // count == 0 означає padding-рядок (фіктивний), тобто насправді не знайдено
            return count > 0;
        }
    }

    // Суфікс не знайдено у відповіді — пароль чистий
    outCount = 0;
    return false;
}
