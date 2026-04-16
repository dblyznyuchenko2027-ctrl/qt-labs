#include "batchpasswordchecker.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QDebug>
#include <atomic>
#include <functional>

// ---------------------------------------------------------------------------
// Helpers (free functions, не залежать від UI)
// ---------------------------------------------------------------------------

namespace {

/**
 * @brief Синхронна перевірка одного пароля через Pwned Passwords API
 *
 * Виконується у фоновому потоці — блокувальні виклики тут допустимі.
 * QNetworkAccessManager + QEventLoop дозволяють зробити синхронний запит
 * без блокування GUI (бо цей код вже не в GUI-потоці).
 */
SingleCheckResult checkOnePassword(const CredentialCheckTask &task)
{
    SingleCheckResult result;
    result.taskId = task.id;

    if (task.password.isEmpty()) {
        result.status   = LeakStatus::CheckFailed;
        result.errorMsg = "Порожній пароль";
        return result;
    }

    // 1. SHA-1 від пароля — локально, пароль не залишить пристрій
    const QByteArray sha1 = QCryptographicHash::hash(
                                task.password.toUtf8(),
                                QCryptographicHash::Sha1
                            ).toHex().toUpper();

    const QByteArray prefix = sha1.left(5);
    const QByteArray suffix = sha1.mid(5);

    // 2. Формуємо запит
    const QString urlStr = QStringLiteral("https://api.pwnedpasswords.com/range/")
                           + QString::fromLatin1(prefix);

    QNetworkRequest request{QUrl{urlStr}};
    request.setRawHeader("User-Agent", "PasswordManagerCourse/1.0");
    request.setRawHeader("Add-Padding", "true");
    request.setTransferTimeout(10000);

    // 3. Синхронний запит у фоновому потоці через QEventLoop
    //    (у GUI-потоці так робити не можна!)
    QNetworkAccessManager nam;
    QEventLoop loop;
    QNetworkReply *reply = nam.get(request);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();  // блокує лише цей фоновий потік

    // 4. Обробка відповіді
    if (reply->error() != QNetworkReply::NoError) {
        result.status   = LeakStatus::CheckFailed;
        result.errorMsg = reply->errorString();
        reply->deleteLater();
        return result;
    }

    const int httpStatus = reply->attribute(
                               QNetworkRequest::HttpStatusCodeAttribute
                           ).toInt();

    if (httpStatus != 200) {
        result.status   = LeakStatus::CheckFailed;
        result.errorMsg = QString("HTTP %1").arg(httpStatus);
        reply->deleteLater();
        return result;
    }

    const QByteArray body = reply->readAll();
    reply->deleteLater();

    // 5. Розбір текстового формату SUFFIX:COUNT
    bool found = false;
    int  count = 0;

    for (const QByteArray &line : body.split('\n')) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.isEmpty()) continue;

        const QList<QByteArray> parts = trimmed.split(':');
        if (parts.size() < 2) continue;

        if (parts[0] == suffix) {
            count = parts[1].toInt();
            // count == 0 → padding-рядок, не справжній збіг
            found = (count > 0);
            break;
        }
    }

    result.status    = found ? LeakStatus::Compromised : LeakStatus::Clean;
    result.leakCount = count;
    return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// BatchPasswordChecker
// ---------------------------------------------------------------------------

BatchPasswordChecker::BatchPasswordChecker(QObject *parent)
    : QObject{parent}
{}

BatchPasswordChecker::~BatchPasswordChecker()
{
    cancelCheck();
    // Якщо watcher ще живий — зачекаємо завершення потоку
    if (m_watcher) {
        m_watcher->waitForFinished();
    }
}

void BatchPasswordChecker::startCheck(const QList<CredentialCheckTask> &tasks)
{
    if (isRunning()) {
        qWarning() << "[BatchChecker] Вже виконується — ігноруємо повторний запуск";
        return;
    }

    m_cancelFlag.store(false);

    // Watcher живе до завершення задачі, потім сам видаляється
    m_watcher = new QFutureWatcher<BatchCheckResult>(this);

    connect(m_watcher, &QFutureWatcher<BatchCheckResult>::finished,
            this, &BatchPasswordChecker::onWatcherFinished);

    // Прогрес і одиночні результати повертаємо через invokeMethod,
    // щоб гарантовано потрапити в GUI-потік (queued connection)
    auto progressCb = [this](int checked, int total) {
        QMetaObject::invokeMethod(this, "progressChanged",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, checked),
                                  Q_ARG(int, total));
    };

    auto singleCb = [this](SingleCheckResult r) {
        QMetaObject::invokeMethod(this, "singleResultReady",
                                  Qt::QueuedConnection,
                                  Q_ARG(SingleCheckResult, r));
    };

    // Запускаємо у фоновому потоці через QtConcurrent::run()
    // Лямбда захоплює лише дані, не UI-об'єкти
    std::atomic<bool> *cancelPtr = &m_cancelFlag;

    QFuture<BatchCheckResult> future = QtConcurrent::run(
        [tasks, progressCb, singleCb, cancelPtr]() -> BatchCheckResult {
            return BatchPasswordChecker::runBatchCheck(tasks, progressCb, singleCb, cancelPtr);
        }
    );

    m_watcher->setFuture(future);
    emit started(tasks.size());
}

void BatchPasswordChecker::cancelCheck()
{
    m_cancelFlag.store(true);
}

bool BatchPasswordChecker::isRunning() const
{
    return m_watcher && m_watcher->isRunning();
}

// ---------------------------------------------------------------------------
// Private slot — викликається у GUI-потоці після завершення фонової задачі
// ---------------------------------------------------------------------------

void BatchPasswordChecker::onWatcherFinished()
{
    BatchCheckResult result = m_watcher->result();
    m_watcher->deleteLater();
    m_watcher = nullptr;

    if (m_cancelFlag.load()) {
        emit cancelled();
    } else {
        emit finished(result);
    }
}

// ---------------------------------------------------------------------------
// Static фонова функція — виконується поза GUI-потоком
// ---------------------------------------------------------------------------

BatchCheckResult BatchPasswordChecker::runBatchCheck(
    QList<CredentialCheckTask> tasks,
    std::function<void(int, int)> progressCallback,
    std::function<void(SingleCheckResult)> singleCallback,
    std::atomic<bool> *cancelFlag)
{
    BatchCheckResult batch;
    batch.total = tasks.size();

    for (const CredentialCheckTask &task : tasks) {
        // Перевіряємо прапорець скасування перед кожним записом
        if (cancelFlag && cancelFlag->load()) {
            qDebug() << "[BatchChecker] Скасовано після" << batch.checked << "записів";
            break;
        }

        SingleCheckResult singleResult = checkOnePassword(task);

        ++batch.checked;

        if (singleResult.status == LeakStatus::Compromised) {
            ++batch.compromised;
        } else if (singleResult.status == LeakStatus::CheckFailed) {
            ++batch.failed;
            qWarning() << "[BatchChecker] Помилка для запису" << task.name
                       << ":" << singleResult.errorMsg;
        }

        batch.results.append(singleResult);

        // Надсилаємо прогрес і одиночний результат у GUI-потік
        if (progressCallback) progressCallback(batch.checked, batch.total);
        if (singleCallback)   singleCallback(singleResult);
    }

    return batch;
}
