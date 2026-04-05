#pragma once

#include <QMainWindow>
#include "passwordtablemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewTriggered();
    void onEditTriggered();
    void onDeleteTriggered();
    void onSaveTriggered();
    void onCopyUsernameTriggered();
    void onCopyPasswordTriggered();
    void onSearchTextChanged(const QString &text);
    void onClearSearch();
    void onSelectionChanged();

private:
    Ui::MainWindow       *ui;
    PasswordTableModel   *m_model;

    void setupModel();
    void updateStatusBar();
    void updateActions();
};
