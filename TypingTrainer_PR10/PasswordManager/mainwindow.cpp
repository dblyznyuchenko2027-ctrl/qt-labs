#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QDir>

// ── Constructor / Destructor ───────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dbManager(new DatabaseManager)
    , m_repository(nullptr)
    , m_model(nullptr)
    , m_proxy(nullptr)
{
    ui->setupUi(this);
    setupDatabase();

    if (m_repository) {
        setupModel();
        setupCategoryCombo();
        reloadData();
    }

    // ── Action connections ────────────────────────────────────────────────────
    connect(ui->actionNew,          &QAction::triggered, this, &MainWindow::onNewTriggered);
    connect(ui->actionEdit,         &QAction::triggered, this, &MainWindow::onEditTriggered);
    connect(ui->actionDelete,       &QAction::triggered, this, &MainWindow::onDeleteTriggered);
    connect(ui->actionSave,         &QAction::triggered, this, &MainWindow::onSaveTriggered);
    connect(ui->actionExit,         &QAction::triggered, qApp,  &QApplication::quit);
    connect(ui->actionCopyUsername, &QAction::triggered, this, &MainWindow::onCopyUsernameTriggered);
    connect(ui->actionCopyPassword, &QAction::triggered, this, &MainWindow::onCopyPasswordTriggered);

    // ── Filter / search connections ───────────────────────────────────────────
    connect(ui->lineEditSearch,    &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    connect(ui->pushButtonClear,   &QPushButton::clicked,
            this, &MainWindow::onClearSearch);
    connect(ui->comboBoxCategory,  QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCategoryFilterChanged);

    updateActions();
    updateStatusBar();
    updateEmptyState();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_repository;
    delete m_dbManager;
}

// ── Private setup ─────────────────────────────────────────────────────────────

void MainWindow::setupDatabase()
{
    const QString dbPath = QDir::currentPath() + "/passwords.db";

    if (!m_dbManager->open(dbPath)) {
        QMessageBox::critical(this, "Database Error",
            "Cannot open database:\n" + m_dbManager->lastError());
        return;
    }
    if (!m_dbManager->initializeSchema()) {
        QMessageBox::critical(this, "Database Error",
            "Cannot initialize schema:\n" + m_dbManager->lastError());
        return;
    }
    m_repository = new PasswordRepository(m_dbManager->database());
}

void MainWindow::setupModel()
{
    m_model = new PasswordTableModel(m_repository, this);

    m_proxy = new PasswordFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortRole(Qt::DisplayRole);

    ui->tableViewPasswords->setModel(m_proxy);
    ui->tableViewPasswords->setSortingEnabled(true);
    ui->tableViewPasswords->sortByColumn(PasswordTableModel::ColTitle, Qt::AscendingOrder);

    ui->tableViewPasswords->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewPasswords->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewPasswords->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::SelectedClicked
    );
    ui->tableViewPasswords->setAlternatingRowColors(true);
    ui->tableViewPasswords->verticalHeader()->setVisible(false);
    ui->tableViewPasswords->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewPasswords->setColumnWidth(PasswordTableModel::ColId,        40);
    ui->tableViewPasswords->setColumnWidth(PasswordTableModel::ColTitle,    150);
    ui->tableViewPasswords->setColumnWidth(PasswordTableModel::ColUsername, 130);
    ui->tableViewPasswords->setColumnWidth(PasswordTableModel::ColPassword, 100);
    ui->tableViewPasswords->setColumnWidth(PasswordTableModel::ColWebsite,  160);
    ui->tableViewPasswords->setColumnWidth(PasswordTableModel::ColCategory, 100);

    connect(ui->tableViewPasswords->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
}

void MainWindow::setupCategoryCombo()
{
    // Populated again after every reload
    ui->comboBoxCategory->blockSignals(true);
    ui->comboBoxCategory->clear();
    ui->comboBoxCategory->addItem("All", "");
    for (const QString &cat : m_model->categories())
        ui->comboBoxCategory->addItem(cat, cat);
    ui->comboBoxCategory->blockSignals(false);
}

// ── Helpers ───────────────────────────────────────────────────────────────────

int MainWindow::currentSourceRow() const
{
    QModelIndex proxyIdx = ui->tableViewPasswords->currentIndex();
    if (!proxyIdx.isValid()) return -1;
    return m_proxy->mapToSource(proxyIdx).row();
}

void MainWindow::reloadData()
{
    if (!m_model) return;
    m_model->reload();
    setupCategoryCombo();
    updateStatusBar();
    updateEmptyState();
}

void MainWindow::updateStatusBar()
{
    int total    = m_model  ? m_model->rowCount()  : 0;
    int filtered = m_proxy  ? m_proxy->rowCount()  : 0;
    statusBar()->showMessage(
        QString("Total: %1  Filtered: %2").arg(total).arg(filtered));
}

void MainWindow::updateActions()
{
    bool sel = ui->tableViewPasswords->currentIndex().isValid();
    ui->actionEdit->setEnabled(sel);
    ui->actionDelete->setEnabled(sel);
    ui->actionCopyUsername->setEnabled(sel);
    ui->actionCopyPassword->setEnabled(sel);
}

void MainWindow::updateEmptyState()
{
    bool empty = m_proxy && (m_proxy->rowCount() == 0);
    ui->labelEmpty->setVisible(empty);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onNewTriggered()
{
    if (!m_repository) return;

    PasswordEntry e;
    e.title    = "New Entry";
    e.category = "General";

    if (!m_repository->insert(e)) {
        QMessageBox::warning(this, "Error",
            "Failed to create entry:\n" + m_repository->lastError());
        return;
    }

    reloadData();

    // Select and begin editing the new entry
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (m_model->entryAt(row).id == e.id) {
            QModelIndex srcIdx   = m_model->index(row, PasswordTableModel::ColTitle);
            QModelIndex proxyIdx = m_proxy->mapFromSource(srcIdx);
            ui->tableViewPasswords->setCurrentIndex(proxyIdx);
            ui->tableViewPasswords->scrollTo(proxyIdx);
            ui->tableViewPasswords->edit(proxyIdx);
            break;
        }
    }
    updateActions();
}

void MainWindow::onEditTriggered()
{
    QModelIndex idx = ui->tableViewPasswords->currentIndex();
    if (idx.isValid())
        ui->tableViewPasswords->edit(idx);
}

void MainWindow::onDeleteTriggered()
{
    int srcRow = currentSourceRow();
    if (srcRow < 0) return;

    PasswordEntry e = m_model->entryAt(srcRow);

    auto answer = QMessageBox::question(
        this, "Delete Entry",
        QString("Delete entry \"%1\"?").arg(e.title),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );

    if (answer == QMessageBox::Yes) {
        if (!m_repository->remove(e.id)) {
            QMessageBox::warning(this, "Error",
                "Failed to delete:\n" + m_repository->lastError());
            return;
        }
        reloadData();
        updateActions();
    }
}

void MainWindow::onSaveTriggered()
{
    // Edits are persisted immediately via setData() -> repository.update()
    statusBar()->showMessage("All changes are saved to database.", 3000);
}

void MainWindow::onCopyUsernameTriggered()
{
    int row = currentSourceRow();
    if (row < 0) return;
    QApplication::clipboard()->setText(m_model->entryAt(row).username);
    statusBar()->showMessage("Username copied to clipboard.", 2000);
}

void MainWindow::onCopyPasswordTriggered()
{
    int row = currentSourceRow();
    if (row < 0) return;
    QApplication::clipboard()->setText(m_model->entryAt(row).password);
    statusBar()->showMessage("Password copied to clipboard.", 2000);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    if (m_proxy) {
        m_proxy->setTextFilter(text);
        updateStatusBar();
        updateEmptyState();
    }
}

void MainWindow::onCategoryFilterChanged(int index)
{
    if (!m_proxy) return;
    const QString cat = ui->comboBoxCategory->itemData(index).toString();
    m_proxy->setCategoryFilter(cat);
    updateStatusBar();
    updateEmptyState();
}

void MainWindow::onClearSearch()
{
    ui->lineEditSearch->clear();
    ui->comboBoxCategory->setCurrentIndex(0);
}

void MainWindow::onSelectionChanged()
{
    updateActions();
}
