#pragma once

#include <QMainWindow>
#include "DatabaseManager.h"
#include "MovieRepository.h"
#include "SettingsRepository.h"
#include "MovieService.h"
#include "SettingsManager.h"
#include "MovieTableModel.h"
#include "SearchManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddMovie();
    void onEditMovie();
    void onDeleteMovie();
    void onSearchTextChanged(const QString &text);
    void onStatusFilterChanged(int index);
    void onSelectionChanged();
    void onDoubleClick(const QModelIndex &index);
    void onSettings();

private:
    void setupDatabase();
    void setupModel();
    void setupConnections();
    void reloadData();
    void updateActions();
    void updateStatusBar();
    void applyTheme(const QString &theme);
    int  currentSourceRow() const;

    Ui::MainWindow      *ui;
    DatabaseManager     *m_dbManager    = nullptr;
    MovieRepository     *m_movieRepo    = nullptr;
    SettingsRepository  *m_settingsRepo = nullptr;
    MovieService        *m_movieService = nullptr;
    SettingsManager     *m_settingsMgr  = nullptr;
    MovieTableModel     *m_model        = nullptr;
    SearchManager       *m_proxy        = nullptr;
};
