#ifndef BATCHPASSWORDCHECKER_H
#define BATCHPASSWORDCHECKER_H

#include <QObject>
#include <QFutureWatcher>
#include "batchcheckresult.h"

/**
 * @brief Клас для масової перевірки всіх паролів у фоновому режимі
 *
 * Використовує QtConcurrent::run() для виконання поза GUI-потоком
 * та QFutureWatcher для безпечного повернення результатів у головний потік.
 *
 * Принцип:
 *  - фонова функція не торкається UI
 *  - прогрес і результати надходять через сигнали (queued connection)
 *  - UI оновлюється виключно в головному потоці
 */
class BatchPasswordChecker : public QObject
{
    Q_OBJECT

public:
    explicit BatchPasswordChecker(QObject *parent = nullptr);
    ~BatchPasswordChecker();

    /**
     * @brief Запустити масову перевірку
     * @param tasks Список записів для перевірки
     *
     * Якщо перевірка вже виконується — виклик ігнорується.
     */
    void startCheck(const QList<CredentialCheckTask> &tasks);

    /**
     * @brief Скасувати поточну перевірку
     * Встановлює прапорець скасування; цикл фонової задачі завершується
     * контрольовано після поточного запису.
     */
    void cancelCheck();

    bool isRunning() const;

signals:
    /** Перевірка запущена */
    void started(int totalCount);

    /** Один запис перевірено (викликається з головного потоку) */
    void progressChanged(int checked, int total);

    /** Результат одного запису готовий */
    void singleResultReady(SingleCheckResult result);

    /** Пакетна перевірка повністю завершена */
    void finished(BatchCheckResult result);

    /** Перевірку скасовано користувачем */
    void cancelled();

private slots:
    void onWatcherFinished();

private:
    // Ця функція виконується у фоновому потоці через QtConcurrent::run()
    // Вона НЕ є методом класу, щоб унеможливити прямий доступ до UI
    static BatchCheckResult runBatchCheck(
        QList<CredentialCheckTask> tasks,
        std::function<void(int, int)> progressCallback,
        std::function<void(SingleCheckResult)> singleCallback,
        std::atomic<bool> *cancelFlag
    );

    QFutureWatcher<BatchCheckResult> *m_watcher   = nullptr;
    std::atomic<bool>                 m_cancelFlag{false};
};

#endif // BATCHPASSWORDCHECKER_H
