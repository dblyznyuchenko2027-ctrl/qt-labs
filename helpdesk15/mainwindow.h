#pragma once

#include <QMainWindow>
#include "tickettablemodel.h"
#include "ticketfilterproxymodel.h"
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
    void onClearFiltersTriggered();
    void onTableDoubleClicked(const QModelIndex &proxyIndex);
    void updateActionsState();
    void updateFilters();
    void updateEmptyState();
    void updateStatusBar();

    void onTicketCreateRequested(const Ticket &ticket);
    void onTicketUpdateRequested(const Ticket &ticket);

private:
    int currentSourceRow() const;
    int findNextId() const;
    void loadData();
    void saveData();
    void openViewDialog(int sourceRow);
    void openEditDialog(int sourceRow);

    Ui::MainWindow *ui;
    TicketTableModel        *m_model;
    TicketFilterProxyModel  *m_proxy;
    CsvTicketRepository     *m_repository;
};
