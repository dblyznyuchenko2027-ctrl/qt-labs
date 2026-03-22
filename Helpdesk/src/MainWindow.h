#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QSortFilterProxyModel>
#include "TicketModel.h"
#include "TicketDialog.h"

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
    void onActionNew();
    void onActionView();
    void onActionEdit();
    void onActionDelete();
    void onActionRefresh();
    void onActionAbout();

    void onSelectionChanged();
    void onDoubleClick(const QModelIndex &index);
    void onTicketSaved(const Ticket &t);

    void onFilterChanged();
    void onClearFilter();

private:
    void setupConnections();
    void setupTable();
    void updateActionStates();
    void updateStatusBar();

    int  selectedSourceRow() const;   // -1 if nothing selected
    void openDialog(DialogMode mode, int sourceRow = -1);

    Ui::MainWindow             *ui;
    TicketModel                *m_model      = nullptr;
    QSortFilterProxyModel      *m_proxy      = nullptr;
    QPointer<TicketDialog>      m_dialog;

    // Remember which row is being edited (source model row)
    int m_editingRow = -1;
};
