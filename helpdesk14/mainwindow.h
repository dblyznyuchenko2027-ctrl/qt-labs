#pragma once

#include <QMainWindow>
#include "tickettablemodel.h"
#include "csvticketrepository.h"

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
    void onViewTriggered();
    void onEditTriggered();
    void onDeleteTriggered();
    void onRefreshTriggered();
    void onTableDoubleClicked(const QModelIndex &index);
    void updateActionsState();

    void onTicketCreateRequested(const Ticket &ticket);
    void onTicketUpdateRequested(const Ticket &ticket);

private:
    int currentRow() const;
    int findNextId() const;
    void loadData();
    void saveData();
    void updateStatusBar();

    Ui::MainWindow *ui;
    TicketTableModel *m_model;
    CsvTicketRepository *m_repository;
};
