#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ticketdialog.h"

#include <QMessageBox>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new TicketTableModel(this))
    , m_repository(new CsvTicketRepository(
          QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
          + "/tickets.csv"))
{
    ui->setupUi(this);

    ui->tableView->setModel(m_model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, &MainWindow::updateActionsState);

    connect(ui->tableView, &QTableView::doubleClicked,
            this, &MainWindow::onTableDoubleClicked);

    connect(ui->actionNew,     &QAction::triggered, this, &MainWindow::onNewTriggered);
    connect(ui->actionView,    &QAction::triggered, this, &MainWindow::onViewTriggered);
    connect(ui->actionEdit,    &QAction::triggered, this, &MainWindow::onEditTriggered);
    connect(ui->actionDelete,  &QAction::triggered, this, &MainWindow::onDeleteTriggered);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::onRefreshTriggered);
    connect(ui->actionExit,    &QAction::triggered, this, &QMainWindow::close);

    loadData();
    updateActionsState();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::currentRow() const
{
    const auto rows = ui->tableView->selectionModel()->selectedRows();
    if (rows.isEmpty())
        return -1;
    return rows.first().row();
}

int MainWindow::findNextId() const
{
    int maxId = 0;
    for (const Ticket &t : m_model->tickets())
    {
        if (t.id > maxId)
            maxId = t.id;
    }
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

void MainWindow::updateActionsState()
{
    const bool hasSelection = ui->tableView->selectionModel()->hasSelection();
    ui->actionNew->setEnabled(true);
    ui->actionView->setEnabled(hasSelection);
    ui->actionEdit->setEnabled(hasSelection);
    ui->actionDelete->setEnabled(hasSelection);
    ui->actionRefresh->setEnabled(true);
}

void MainWindow::updateStatusBar()
{
    ui->statusbar->showMessage(
        QString("Total: %1").arg(m_model->rowCount()));
}

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
    const int row = currentRow();
    if (row < 0)
        return;

    auto *dialog = new TicketDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMode(TicketDialog::Mode::View);
    dialog->loadTicket(m_model->ticketAt(row));

    connect(dialog, &TicketDialog::updateRequested,
            this, &MainWindow::onTicketUpdateRequested);

    dialog->show();
}

void MainWindow::onEditTriggered()
{
    const int row = currentRow();
    if (row < 0)
        return;

    auto *dialog = new TicketDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setMode(TicketDialog::Mode::Edit);
    dialog->loadTicket(m_model->ticketAt(row));

    connect(dialog, &TicketDialog::updateRequested,
            this, &MainWindow::onTicketUpdateRequested);

    dialog->show();
}

void MainWindow::onDeleteTriggered()
{
    const int row = currentRow();
    if (row < 0)
        return;

    const auto result = QMessageBox::question(
        this,
        "Delete Ticket",
        "Do you really want to delete the selected ticket?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (result == QMessageBox::Yes)
    {
        m_model->removeTicket(row);
        saveData();
        updateActionsState();
        updateStatusBar();
    }
}

void MainWindow::onRefreshTriggered()
{
    loadData();
    updateActionsState();
    updateStatusBar();
}

void MainWindow::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    onViewTriggered();
}

void MainWindow::onTicketCreateRequested(const Ticket &ticket)
{
    Ticket newTicket = ticket;
    newTicket.id = findNextId();
    newTicket.createdAt = QDateTime::currentDateTime();
    m_model->appendTicket(newTicket);
    saveData();
    updateStatusBar();
}

void MainWindow::onTicketUpdateRequested(const Ticket &ticket)
{
    const auto &items = m_model->tickets();
    for (int i = 0; i < items.size(); ++i)
    {
        if (items[i].id == ticket.id)
        {
            Ticket updated = ticket;
            updated.createdAt = items[i].createdAt;
            m_model->replaceTicket(i, updated);
            saveData();
            break;
        }
    }
}
