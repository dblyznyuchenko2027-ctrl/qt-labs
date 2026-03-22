#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QSortFilterProxyModel>
#include "TicketTableModel.h"
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
    // Ticket CRUD commands
    void onActionNew();
    void onActionView();
    void onActionEdit();
    void onActionDelete();
    void onActionRefresh();
    void onActionAbout();

    // Table selection
    void onSelectionChanged();
    void onDoubleClick(const QModelIndex &index);

    // Dialog callback
    void onTicketSaved(const Ticket &t);

    // Filter bar
    void onFilterChanged();
    void onClearFilter();

private:
    void setupTable();
    void setupConnections();
    void updateActionStates();
    void updateStatusBar();

    // Returns source-model row of current selection, or -1
    int  selectedSourceRow() const;
    void openDialog(DialogMode mode, int sourceRow = -1);

    Ui::MainWindow        *ui;
    TicketTableModel      *m_model  = nullptr;   // owns data
    QSortFilterProxyModel *m_proxy  = nullptr;   // sorting + text search
    QPointer<TicketDialog> m_dialog;             // non-modal, reused

    int m_editingRow = -1;  // source row currently being edited (-1 = New)
};
