#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include "passwordleakchecker.h"   // ПР19: перевірка одного пароля
#include "batchpasswordchecker.h"  // ПР20: масова фонова перевірка
#include "batchcheckresult.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PasswordTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- Дії з записами ---
    void onNewEntry();
    void onEditEntry();
    void onDeleteEntry();
    void onCopyUsername();
    void onCopyPassword();

    // --- Пошук / фільтр ---
    void onSearchChanged(const QString &text);
    void onClearSearch();
    void onCategoryFilterChanged(int index);

    // --- ПР19: перевірка одного пароля ---
    void onCheckPasswordLeaked();
    void onLeakCheckCompleted(bool found, int count);
    void onLeakCheckFailed(const QString &message);
    void onLeakCheckingStateChanged(bool isChecking);

    // --- ПР20: масова фонова перевірка ---
    void onCheckAllPasswords();
    void onCancelBatchCheck();
    void onBatchStarted(int totalCount);
    void onBatchProgressChanged(int checked, int total);
    void onBatchSingleResultReady(SingleCheckResult result);
    void onBatchFinished(BatchCheckResult result);
    void onBatchCancelled();

private:
    void setupModel();
    void setupConnections();
    void setBatchCheckRunning(bool running);
    void updateStatusBar();
    void applyLeakStatusToModel(const SingleCheckResult &result);

    QString currentSelectedPassword() const;
    QList<CredentialCheckTask> collectAllTasks() const;

    Ui::MainWindow        *ui            = nullptr;
    PasswordTableModel    *m_model       = nullptr;
    QSortFilterProxyModel *m_proxyModel  = nullptr;

    PasswordLeakChecker   *m_leakChecker  = nullptr;  // ПР19
    BatchPasswordChecker  *m_batchChecker = nullptr;  // ПР20
};

#endif // MAINWINDOW_H
