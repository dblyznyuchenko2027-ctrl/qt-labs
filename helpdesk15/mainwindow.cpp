#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ticketdialog.h"

#include <QMessageBox>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new TicketTableModel(this))
    , m_proxy(new TicketFilterProxyModel(this))
    , m_repository(new CsvTicketRepository(
          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
          + "/tickets.csv"))
{
    ui->setupUi(this);

    // Connect proxy to source model and table
    m_proxy->setSourceModel(m_model);
    ui->tableView->setModel(m_proxy);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->sortByColumn(0, Qt::AscendingOrder);

    // Selection changes
    connect(ui->tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateActionsState);

    // Double-click opens view dialog
    connect(ui->tableView, &QTableView::doubleClicked,
            this, &MainWindow::onTableDoubleClicked);

    // Filter controls
    connect(ui->searchEdit,   &QLineEdit::textChanged,
            this, &MainWindow::updateFilters);
    connect(ui->comboStatus,  &QComboBox::currentTextChanged,
            this, &MainWindow::updateFilters);
    connect(ui->comboPriority, &QComboBox::currentTextChanged,
            this, &MainWindow::updateFilters);
    connect(ui->btnClearFilters, &QPushButton::clicked,
            this, &MainWindow::onClearFiltersTriggered);

    // Toolbar / menu actions
    connect(ui->actionNew,     &QAction::triggered, this, &MainWindow::onNewTriggered);
    connect(ui->actionView,    &QAction::triggered, this, &MainWindow::onViewTriggered);
    connect(ui->actionEdit,    &QAction::triggered, this, &MainWindow::onEditTriggered);
    connect(ui->actionDelete,  &QAction::triggered, this, &MainWindow::onDeleteTriggered);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::onRefreshTriggered);
    connect(ui->actionExit,    &QAction::triggered, this, &QMainWindow::close);

    // Update empty state when proxy row count changes
    connect(m_proxy, &QAbstractItemModel::rowsInserted,
            this, &MainWindow::updateEmptyState);
    connect(m_proxy, &QAbstractItemModel::rowsRemoved,
            this, &MainWindow::updateEmptyState);
    connect(m_proxy, &QAbstractItemModel::modelReset,
            this, &MainWindow::updateEmptyState);

    loadData();
    updateActionsState();
    updateEmptyState();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

int MainWindow::currentSourceRow() const
{
    const auto rows = ui->tableView->selectionModel()->selectedRows();
    if (rows.isEmpty())
        return -1;
    const QModelIndex proxyIndex  = rows.first();
    const QModelIndex sourceIndex = m_proxy->mapToSource(proxyIndex);
    return sourceIndex.row();
}

int MainWindow::findNextId() const
{
    int maxId = 0;
    for (const Ticket &t : m_model->tickets())
        if (t.id > maxId)
            maxId = t.id;
    return maxId + 1;
}

void MainWindow::loadData()
{
    const QList<Ticket> items = m_repository->loadAll();
    m_model->setItems(items);
}

void MainWindow::saveData()
{
    m_repository->saveAll(m_model->tickets());
}

void MainWindow::openViewDialog(int sourceRow)
{
    auto *dialog = new TicketDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMode(TicketDialog::Mode::View);
    dialog->loadTicket(m_model->ticketAt(sourceRow));
    connect(dialog, &TicketDialog::updateRequested,
            this, &MainWindow::onTicketUpdateRequested);
    dialog->show();
}

void MainWindow::openEditDialog(int sourceRow)
{
    auto *dialog = new TicketDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMode(TicketDialog::Mode::Edit);
    dialog->loadTicket(m_model->ticketAt(sourceRow));
    connect(dialog, &TicketDialog::updateRequested,
            this, &MainWindow::onTicketUpdateRequested);
    dialog->show();
}

// ---------------------------------------------------------------------------
// UI update
// ---------------------------------------------------------------------------

void MainWindow::updateActionsState()
{
    const bool hasSelection = ui->tableView->selectionModel()->hasSelection();
    ui->actionNew->setEnabled(true);
    ui->actionView->setEnabled(hasSelection);
    ui->actionEdit->setEnabled(hasSelection);
    ui->actionDelete->setEnabled(hasSelection);
    ui->actionRefresh->setEnabled(true);
}

void MainWindow::updateFilters()
{
    const QString text     = ui->searchEdit->text().trimmed();
    const QString status   = (ui->comboStatus->currentIndex() == 0)
                             ? QString() : ui->comboStatus->currentText();
    const QString priority = (ui->comboPriority->currentIndex() == 0)
                             ? QString() : ui->comboPriority->currentText();

    m_proxy->setTextFilter(text);
    m_proxy->setStatusFilter(status);
    m_proxy->setPriorityFilter(priority);

    updateEmptyState();
    updateStatusBar();
}

void MainWindow::updateEmptyState()
{
    const bool empty = (m_proxy->rowCount() == 0);
    ui->stackedWidget->setCurrentIndex(empty ? 1 : 0);
    updateActionsState();
}

void MainWindow::updateStatusBar()
{
    ui->statusbar->showMessage(
        QString("Total: %1  |  Filtered: %2")
            .arg(m_model->rowCount())
            .arg(m_proxy->rowCount()));
}

// ---------------------------------------------------------------------------
// Slots — actions
// ---------------------------------------------------------------------------

void MainWindow::onNewTriggered()
{
    auto *dialog = new TicketDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMode(TicketDialog::Mode::Create);
    connect(dialog, &TicketDialog::createRequested,
            this, &MainWindow::onTicketCreateRequested);
    dialog->show();
}

void MainWindow::onViewTriggered()
{
    const int row = currentSourceRow();
    if (row < 0) return;
    openViewDialog(row);
}

void MainWindow::onEditTriggered()
{
    const int row = currentSourceRow();
    if (row < 0) return;
    openEditDialog(row);
}

void MainWindow::onDeleteTriggered()
{
    const int row = currentSourceRow();
    if (row < 0) return;

    const auto result = QMessageBox::question(
        this,
        "Delete Ticket",
        "Do you really want to delete the selected ticket?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result == QMessageBox::Yes)
    {
        m_model->removeTicket(row);
        saveData();
        updateEmptyState();
        updateStatusBar();
    }
}

void MainWindow::onRefreshTriggered()
{
    loadData();
    updateEmptyState();
    updateStatusBar();
}

void MainWindow::onClearFiltersTriggered()
{
    ui->searchEdit->clear();
    ui->comboStatus->setCurrentIndex(0);
    ui->comboPriority->setCurrentIndex(0);
    // updateFilters() will be triggered by signal chain above
}

void MainWindow::onTableDoubleClicked(const QModelIndex &proxyIndex)
{
    if (!proxyIndex.isValid()) return;
    const QModelIndex sourceIndex = m_proxy->mapToSource(proxyIndex);
    openViewDialog(sourceIndex.row());
}

// ---------------------------------------------------------------------------
// Slots — data changes from dialog
// ---------------------------------------------------------------------------

void MainWindow::onTicketCreateRequested(const Ticket &ticket)
{
    Ticket newTicket    = ticket;
    newTicket.id        = findNextId();
    newTicket.createdAt = QDateTime::currentDateTime();
    m_model->appendTicket(newTicket);
    saveData();
    updateEmptyState();
    updateStatusBar();
}

void MainWindow::onTicketUpdateRequested(const Ticket &ticket)
{
    const auto &items = m_model->tickets();
    for (int i = 0; i < items.size(); ++i)
    {
        if (items[i].id == ticket.id)
        {
            Ticket updated    = ticket;
            updated.createdAt = items[i].createdAt;
            m_model->replaceTicket(i, updated);
            saveData();
            break;
        }
    }
}
