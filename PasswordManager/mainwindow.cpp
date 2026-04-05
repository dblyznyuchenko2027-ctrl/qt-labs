#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new PasswordTableModel(this))
{
    ui->setupUi(this);
    setupModel();

    // Actions
    connect(ui->actionNew,          &QAction::triggered, this, &MainWindow::onNewTriggered);
    connect(ui->actionEdit,         &QAction::triggered, this, &MainWindow::onEditTriggered);
    connect(ui->actionDelete,       &QAction::triggered, this, &MainWindow::onDeleteTriggered);
    connect(ui->actionSave,         &QAction::triggered, this, &MainWindow::onSaveTriggered);
    connect(ui->actionExit,         &QAction::triggered, qApp,  &QApplication::quit);
    connect(ui->actionCopyUsername, &QAction::triggered, this, &MainWindow::onCopyUsernameTriggered);
    connect(ui->actionCopyPassword, &QAction::triggered, this, &MainWindow::onCopyPasswordTriggered);

    // Search
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(ui->pushButtonClear, &QPushButton::clicked, this, &MainWindow::onClearSearch);

    // Selection
    connect(ui->tableViewPasswords->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    updateStatusBar();
    updateActions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

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
    ui->tableViewPasswords->setColumnWidth(1, 140);  // Title
    ui->tableViewPasswords->setColumnWidth(2, 120);  // Username
    ui->tableViewPasswords->setColumnWidth(3, 100);  // Password
    ui->tableViewPasswords->setColumnWidth(4, 160);  // Website
    ui->tableViewPasswords->setColumnWidth(5, 100);  // Category
}

void MainWindow::onNewTriggered()
{
    PasswordEntry e;
    e.title    = "New Entry";
    e.category = "General";
    m_model->addEntry(e);

    int newRow = m_model->rowCount() - 1;
    QModelIndex idx = m_model->index(newRow, 1); // Title column
    ui->tableViewPasswords->setCurrentIndex(idx);
    ui->tableViewPasswords->scrollTo(idx);
    ui->tableViewPasswords->edit(idx);

    updateStatusBar();
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
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;

    const auto answer = QMessageBox::question(
        this,
        "Delete Entry",
        "Delete selected entry?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );

    if (answer == QMessageBox::Yes) {
        m_model->removeEntry(current.row());
        updateStatusBar();
        updateActions();
    }
}

void MainWindow::onSaveTriggered()
{
    // Placeholder for future SQLite persistence
    QMessageBox::information(this, "Save", "Save functionality will be implemented in the next practical work.");
}

void MainWindow::onCopyUsernameTriggered()
{
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;
    PasswordEntry e = m_model->entryAt(current.row());
    QApplication::clipboard()->setText(e.username);
    statusBar()->showMessage("Username copied to clipboard", 2000);
}

void MainWindow::onCopyPasswordTriggered()
{
    QModelIndex current = ui->tableViewPasswords->currentIndex();
    if (!current.isValid()) return;
    PasswordEntry e = m_model->entryAt(current.row());
    QApplication::clipboard()->setText(e.password);
    statusBar()->showMessage("Password copied to clipboard", 2000);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    // Placeholder for proxy model filtering in next practical work
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

void MainWindow::updateStatusBar()
{
    int total = m_model->rowCount();
    ui->statusbar->showMessage(QString("Total: %1  Filtered: %1").arg(total));
}

void MainWindow::updateActions()
{
    bool hasSelection = ui->tableViewPasswords->currentIndex().isValid();
    ui->actionEdit->setEnabled(hasSelection);
    ui->actionDelete->setEnabled(hasSelection);
    ui->actionCopyUsername->setEnabled(hasSelection);
    ui->actionCopyPassword->setEnabled(hasSelection);
}
