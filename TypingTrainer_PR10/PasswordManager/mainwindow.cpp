#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QStandardPaths>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new PasswordTableModel(this))
    , m_dbManager(new DatabaseManager)
    , m_repository(nullptr)
{
    ui->setupUi(this);
    setupDatabase();
    setupModel();

    // Actions
    connect(ui->actionNew,          &QAction::triggered, this, &MainWindow::onNewTriggered);
    connect(ui->actionEdit,         &QAction::triggered, this, &MainWindow::onEditTriggered);
    connect(ui->actionDelete,       &QAction::triggered, this, &MainWindow::onDeleteTriggered);
    connect(ui->actionSave,         &QAction::triggered, this, &MainWindow::onSaveTriggered);
    connect(ui->actionExit,         &QAction::triggered, qApp,  &QApplication::quit);
    connect(ui->actionCopyUsername, &QAction::triggered, this, &MainWindow::onCopyUsernameTriggered);
    connect(ui->actionCopyPassword, &QAction::triggered, this, &MainWindow::onCopyPasswordTriggered);

    // Search / filter
    connect(ui->lineEditSearch,  &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(ui->pushButtonClear, &QPushButton::clicked,   this, &MainWindow::onClearSearch);

    // In-place cell editing -> persist to DB
    connect(m_model, &PasswordTableModel::entryEdited, this, &MainWindow::onEntryEdited);

    // Selection changes
    connect(ui->tableViewPasswords->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    reloadData();
    updateStatusBar();
    updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_repository;
    delete m_dbManager;
}

// ── Database ──────────────────────────────────────────────────────────────────

void MainWindow::setupDatabase()
{
    // Store the DB file next to the executable (or in app data dir)
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

// ── Model / View ──────────────────────────────────────────────────────────────

void MainWindow::setupModel()
{
    ui->tableViewPasswords->setModel(m_model);
    ui->tableViewPasswords->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewPasswords->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewPasswords->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::SelectedClicked
    );
    ui->tableViewPasswords->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewPasswords->verticalHeader()->setVisible(false);
    ui->tableViewPasswords->setColumnWidth(0, 40);   // ID
    ui->tableViewPasswords->setColumnWidth(1, 150);  // Title
    ui->tableViewPasswords->setColumnWidth(2, 130);  // Username
    ui->tableViewPasswords->setColumnWidth(3, 100);  // Password
    ui->tableViewPasswords->setColumnWidth(4, 160);  // Website
    ui->tableViewPasswords->setColumnWidth(5, 100);  // Category
}

void MainWindow::reloadData()
{
    if (!m_repository) return;
    m_model->setEntries(m_repository->loadAll());
    updateStatusBar();
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

    // Find the new row and start editing Title column
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (m_model->entryAt(row).id == e.id) {
            QModelIndex idx = m_model->index(row, PasswordTableModel::ColTitle);
            ui->tableViewPasswords->setCurrentIndex(idx);
            ui->tableViewPasswords->scrollTo(idx);
            ui->tableViewPasswords->edit(idx);
            break;
        }
    }
    updateActions();
}

void MainWindow::onEditTriggered()
{
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;
    ui->tableViewPasswords->edit(current);
}

void MainWindow::onDeleteTriggered()
{
    if (!m_repository) return;
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;

    PasswordEntry e = m_model->entryAt(current.row());

    const auto answer = QMessageBox::question(
        this,
        "Delete Entry",
        QString("Delete entry \"%1\"?").arg(e.title),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer == QMessageBox::Yes) {
        if (!m_repository->remove(e.id)) {
            QMessageBox::warning(this, "Error",
                "Failed to delete entry:\n" + m_repository->lastError());
            return;
        }
        reloadData();
        updateActions();
    }
}

void MainWindow::onSaveTriggered()
{
    // All edits are persisted immediately via onEntryEdited.
    // Save here shows confirmation to the user.
    statusBar()->showMessage("All changes are saved to database.", 3000);
}

void MainWindow::onEntryEdited(const PasswordEntry &entry)
{
    if (!m_repository) return;

    if (!m_repository->update(entry)) {
        QMessageBox::warning(this, "Error",
            "Failed to save changes:\n" + m_repository->lastError());
    }
    // Refresh the row so Updated At column updates visually
    reloadData();
}

void MainWindow::onCopyUsernameTriggered()
{
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;
    PasswordEntry e = m_model->entryAt(current.row());
    QApplication::clipboard()->setText(e.username);
    statusBar()->showMessage("Username copied to clipboard.", 2000);
}

void MainWindow::onCopyPasswordTriggered()
{
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;
    PasswordEntry e = m_model->entryAt(current.row());
    QApplication::clipboard()->setText(e.password);
    statusBar()->showMessage("Password copied to clipboard.", 2000);
}

void MainWindow::onSearchTextChanged(const QString &/*text*/)
{
    // Placeholder: proxy-model filtering will be added in a later practical
    updateStatusBar();
}

void MainWindow::onClearSearch()
{
    ui->lineEditSearch->clear();
}

void MainWindow::onSelectionChanged()
{
    updateActions();
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void MainWindow::updateStatusBar()
{
    int total = m_model->rowCount();
    statusBar()->showMessage(QString("Total: %1  Filtered: %1").arg(total));
}

void MainWindow::updateActions()
{
    bool hasSelection = ui->tableViewPasswords->currentIndex().isValid();
    ui->actionEdit->setEnabled(hasSelection);
    ui->actionDelete->setEnabled(hasSelection);
    ui->actionCopyUsername->setEnabled(hasSelection);
    ui->actionCopyPassword->setEnabled(hasSelection);
}
