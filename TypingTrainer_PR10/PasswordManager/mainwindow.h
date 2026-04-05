#pragma once

#include <QMainWindow>
#include "passwordtablemodel.h"
#include "passwordfilterproxymodel.h"
#include "databasemanager.h"
#include "passwordrepository.h"

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
    void onCategoryFilterChanged(int index);
    void onClearSearch();
    void onSelectionChanged();

private:
    Ui::MainWindow            *ui;
    DatabaseManager           *m_dbManager;
    PasswordRepository        *m_repository;
    PasswordTableModel        *m_model;
    PasswordFilterProxyModel  *m_proxy;

    void setupDatabase();
    void setupModel();
    void setupCategoryCombo();

    // Returns the source-model row for the currently selected proxy row, or -1
    int currentSourceRow() const;

    void reloadData();
    void updateStatusBar();
    void updateActions();
    void updateEmptyState();
};
