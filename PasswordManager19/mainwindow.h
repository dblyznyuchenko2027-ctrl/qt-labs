#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include "passwordleakchecker.h"

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
    // Дії з записами
    void onNewEntry();
    void onEditEntry();
    void onDeleteEntry();
    void onCopyUsername();
    void onCopyPassword();

    // Пошук і фільтрація
    void onSearchChanged(const QString &text);
    void onClearSearch();
    void onCategoryFilterChanged(int index);

    // Мережева перевірка пароля
    void onCheckPasswordLeaked();
    void onLeakCheckCompleted(bool found, int count);
    void onLeakCheckFailed(const QString &message);
    void onLeakCheckingStateChanged(bool isChecking);

private:
    void setupUi();
    void setupConnections();
    void setupModel();
    void updateStatusBar();
    QString currentSelectedPassword() const;

    Ui::MainWindow          *ui;
    PasswordTableModel      *m_model        = nullptr;
    QSortFilterProxyModel   *m_proxyModel   = nullptr;
    PasswordLeakChecker     *m_leakChecker  = nullptr;
};

#endif // MAINWINDOW_H
