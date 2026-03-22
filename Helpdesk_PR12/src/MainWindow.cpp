#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QToolBar>

// ── Constructor ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Build toolbar
    QToolBar *tb = addToolBar(tr("Main"));
    tb->setObjectName("mainToolBar");
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tb->addAction(ui->actionNew);
    tb->addAction(ui->actionView);
    tb->addAction(ui->actionEdit);
    tb->addAction(ui->actionDelete);
    tb->addSeparator();
    tb->addAction(ui->actionRefresh);

    // 1. Create model  2. Setup table  3. Connect signals
    setupTable();
    setupConnections();

    updateActionStates();
    updateStatusBar();
}

MainWindow::~MainWindow() { delete ui; }

// ── Table setup (PR12 core) ───────────────────────────────────────────────────
void MainWindow::setupTable()
{
    // --- PR12 step 4: create TicketTableModel and connect to QTableView ---
    m_model = new TicketTableModel(this);

    // Proxy model for sorting and text-search filtering
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);  // search across all columns

    // Attach proxy (not model directly) so sorting works out of the box
    ui->tableTickets->setModel(m_proxy);

    // --- PR12 step 7: configure selection behaviour ---
    ui->tableTickets->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableTickets->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableTickets->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableTickets->setAlternatingRowColors(true);
    ui->tableTickets->setSortingEnabled(true);
    ui->tableTickets->sortByColumn(COL_ID, Qt::AscendingOrder);
    ui->tableTickets->verticalHeader()->setVisible(false);

    // --- PR12 step 10: column widths ---
    QHeaderView *hdr = ui->tableTickets->horizontalHeader();
    hdr->setSectionResizeMode(COL_TITLE,    QHeaderView::Stretch);
    hdr->setSectionResizeMode(COL_ID,       QHeaderView::ResizeToContents);
    hdr->setSectionResizeMode(COL_PRIORITY, QHeaderView::ResizeToContents);
    hdr->setSectionResizeMode(COL_STATUS,   QHeaderView::ResizeToContents);
    hdr->setSectionResizeMode(COL_CREATED,  QHeaderView::ResizeToContents);
}

// ── Connections ───────────────────────────────────────────────────────────────
void MainWindow::setupConnections()
{
    // Ticket commands
    connect(ui->actionNew,     &QAction::triggered, this, &MainWindow::onActionNew);
    connect(ui->actionView,    &QAction::triggered, this, &MainWindow::onActionView);
    connect(ui->actionEdit,    &QAction::triggered, this, &MainWindow::onActionEdit);
    connect(ui->actionDelete,  &QAction::triggered, this, &MainWindow::onActionDelete);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::onActionRefresh);
    connect(ui->actionAbout,   &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->actionExit,    &QAction::triggered, this, &QWidget::close);

    // --- PR12 step 9: react to selection changes ---
    // selectionModel() is valid only after setModel()
    connect(ui->tableTickets->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(ui->tableTickets, &QTableView::doubleClicked,
            this, &MainWindow::onDoubleClick);

    // Filter bar
    connect(ui->editSearch,          &QLineEdit::textChanged,
            this, &MainWindow::onFilterChanged);
    connect(ui->comboStatusFilter,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);
    connect(ui->comboPriorityFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);
    connect(ui->btnClearFilter, &QPushButton::clicked, this, &MainWindow::onClearFilter);
}

// ── Selected source row ───────────────────────────────────────────────────────
int MainWindow::selectedSourceRow() const
{
    // Get proxy selection, map back to source model
    const auto rows = ui->tableTickets->selectionModel()->selectedRows();
    if (rows.isEmpty()) return -1;
    return m_proxy->mapToSource(rows.first()).row();
}

// ── Non-modal dialog ──────────────────────────────────────────────────────────
void MainWindow::openDialog(DialogMode mode, int sourceRow)
{
    // Reuse single instance (non-modal: show(), not exec())
    if (!m_dialog)
        m_dialog = new TicketDialog(this);

    // Reconnect each time to avoid duplicate connections
    disconnect(m_dialog, &TicketDialog::ticketSaveRequested,
               this,     &MainWindow::onTicketSaved);
    connect(m_dialog, &TicketDialog::ticketSaveRequested,
            this,     &MainWindow::onTicketSaved);

    m_editingRow = sourceRow;

    switch (mode) {
    case DialogMode::New:
        m_dialog->setModeNew(m_model->nextId());
        break;
    case DialogMode::View:
        m_dialog->setModeView(m_model->ticketAt(sourceRow));
        break;
    case DialogMode::Edit:
        m_dialog->setModeEdit(m_model->ticketAt(sourceRow));
        break;
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

// ── ticketSaveRequested ───────────────────────────────────────────────────────
void MainWindow::onTicketSaved(const Ticket &t)
{
    if (m_editingRow < 0) {
        // --- PR12 step 8: addTicket triggers beginInsertRows/endInsertRows ---
        Ticket newT  = t;
        newT.id      = m_model->nextId();
        newT.createdAt = QDateTime::currentDateTime();
        m_model->addTicket(newT);
        statusBar()->showMessage(tr("Ticket #%1 created.").arg(newT.id), 4000);
    } else {
        // --- PR12 step 8: updateTicket triggers dataChanged ---
        m_model->updateTicket(m_editingRow, t);
        statusBar()->showMessage(tr("Ticket #%1 updated.").arg(t.id), 4000);
    }
    updateStatusBar();
}

// ── Action slots ──────────────────────────────────────────────────────────────
void MainWindow::onActionNew()
{
    m_editingRow = -1;
    openDialog(DialogMode::New);
}

void MainWindow::onActionView()
{
    const int row = selectedSourceRow();
    if (row < 0) return;
    openDialog(DialogMode::View, row);
}

void MainWindow::onActionEdit()
{
    const int row = selectedSourceRow();
    if (row < 0) return;
    openDialog(DialogMode::Edit, row);
}

void MainWindow::onActionDelete()
{
    const int row = selectedSourceRow();
    if (row < 0) return;

    const Ticket t = m_model->ticketAt(row);
    const auto reply = QMessageBox::question(
        this, tr("Delete Ticket"),
        tr("Delete ticket #%1 \"%2\"?\nThis cannot be undone.")
        .arg(t.id).arg(t.title),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_dialog && m_dialog->isVisible() && m_editingRow == row)
            m_dialog->hide();
        // --- PR12 step 8: removeTicket triggers beginRemoveRows/endRemoveRows ---
        m_model->removeTicket(row);
        statusBar()->showMessage(tr("Ticket #%1 deleted.").arg(t.id), 4000);
        updateActionStates();
        updateStatusBar();
    }
}

void MainWindow::onActionRefresh()
{
    m_proxy->invalidate();
    updateStatusBar();
    statusBar()->showMessage(tr("Refreshed."), 2000);
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, tr("About Helpdesk"),
        tr("<b>Helpdesk</b> v1.0<br><br>"
           "Practical work #12 — QAbstractTableModel, Model/View,<br>"
           "beginInsertRows/endInsertRows, dataChanged, "
           "QSortFilterProxyModel."));
}

// ── Selection ─────────────────────────────────────────────────────────────────
void MainWindow::onSelectionChanged()
{
    updateActionStates();
}

void MainWindow::onDoubleClick(const QModelIndex &index)
{
    if (!index.isValid()) return;
    openDialog(DialogMode::View, m_proxy->mapToSource(index).row());
}

// ── Filter ────────────────────────────────────────────────────────────────────
void MainWindow::onFilterChanged()
{
    // Text search across all columns
    m_proxy->setFilterWildcard(ui->editSearch->text().trimmed());
    m_proxy->setFilterKeyColumn(-1);
    updateStatusBar();
}

void MainWindow::onClearFilter()
{
    ui->editSearch->clear();
    ui->comboStatusFilter->setCurrentIndex(0);
    ui->comboPriorityFilter->setCurrentIndex(0);
    m_proxy->setFilterWildcard(QString());
    updateStatusBar();
}

// ── State helpers ─────────────────────────────────────────────────────────────
void MainWindow::updateActionStates()
{
    const bool hasSel = selectedSourceRow() >= 0;
    ui->actionView->setEnabled(hasSel);
    ui->actionEdit->setEnabled(hasSel);
    ui->actionDelete->setEnabled(hasSel);
}

void MainWindow::updateStatusBar()
{
    statusBar()->showMessage(
        tr("Total: %1  |  Filtered: %2")
        .arg(m_model->count())
        .arg(m_proxy->rowCount()));
}
