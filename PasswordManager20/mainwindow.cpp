#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "passwordtablemodel.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}, ui{new Ui::MainWindow}
{
    ui->setupUi(this);
    setupModel();

    // PR19: single password check
    m_leakChecker = new PasswordLeakChecker(this);
    connect(m_leakChecker, &PasswordLeakChecker::checkCompleted, this, &MainWindow::onLeakCheckCompleted);
    connect(m_leakChecker, &PasswordLeakChecker::checkFailed,    this, &MainWindow::onLeakCheckFailed);
    connect(m_leakChecker, &PasswordLeakChecker::checkingStateChanged, this, &MainWindow::onLeakCheckingStateChanged);

    // PR20: batch background check
    m_batchChecker = new BatchPasswordChecker(this);
    connect(m_batchChecker, &BatchPasswordChecker::started,         this, &MainWindow::onBatchStarted);
    connect(m_batchChecker, &BatchPasswordChecker::progressChanged, this, &MainWindow::onBatchProgressChanged);
    connect(m_batchChecker, &BatchPasswordChecker::singleResultReady, this, &MainWindow::onBatchSingleResultReady);
    connect(m_batchChecker, &BatchPasswordChecker::finished,        this, &MainWindow::onBatchFinished);
    connect(m_batchChecker, &BatchPasswordChecker::cancelled,       this, &MainWindow::onBatchCancelled);

    setupConnections();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    if (m_batchChecker && m_batchChecker->isRunning())
        m_batchChecker->cancelCheck();
    delete ui;
}

void MainWindow::setupModel()
{
    m_model = new PasswordTableModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);
    ui->tableView->setModel(m_proxyModel);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSortingEnabled(true);
}

void MainWindow::setupConnections()
{
    connect(ui->actionNewEntry,          &QAction::triggered, this, &MainWindow::onNewEntry);
    connect(ui->actionEditEntry,         &QAction::triggered, this, &MainWindow::onEditEntry);
    connect(ui->actionDeleteEntry,       &QAction::triggered, this, &MainWindow::onDeleteEntry);
    connect(ui->actionCopyUsername,      &QAction::triggered, this, &MainWindow::onCopyUsername);
    connect(ui->actionCopyPassword,      &QAction::triggered, this, &MainWindow::onCopyPassword);
    connect(ui->actionCheckLeaked,       &QAction::triggered, this, &MainWindow::onCheckPasswordLeaked);
    connect(ui->actionCheckAllPasswords, &QAction::triggered, this, &MainWindow::onCheckAllPasswords);
    connect(ui->actionCancelBatchCheck,  &QAction::triggered, this, &MainWindow::onCancelBatchCheck);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchChanged);
    connect(ui->clearSearchButton, &QPushButton::clicked, this, &MainWindow::onClearSearch);
    connect(ui->categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCategoryFilterChanged);
}

// --- Entry actions (stubs) ---
void MainWindow::onNewEntry()    {}
void MainWindow::onEditEntry()   {}
void MainWindow::onDeleteEntry() {}
void MainWindow::onCopyUsername(){}
void MainWindow::onCopyPassword(){}

// --- PR19: single check ---
void MainWindow::onCheckPasswordLeaked()
{
    const QString pw = currentSelectedPassword();
    if (pw.isEmpty()) {
        QMessageBox::information(this, tr("Перевірка"), tr("Оберіть запис із паролем."));
        return;
    }
    m_leakChecker->checkPassword(pw);
}

void MainWindow::onLeakCheckCompleted(bool found, int count)
{
    if (found)
        QMessageBox::warning(this, tr("Пароль знайдено"),
            tr("Пароль знайдено у злитих базах %n раз(ів).", "", count));
    else
        QMessageBox::information(this, tr("Чисто"), tr("Пароль не знайдено у злитих базах."));
    statusBar()->showMessage(found ? tr("Скомпрометовано (%1)").arg(count) : tr("Чисто"), 5000);
}

void MainWindow::onLeakCheckFailed(const QString &msg)
{
    QMessageBox::warning(this, tr("Помилка"), tr("Перевірку не вдалося завершити:\n%1").arg(msg));
}

void MainWindow::onLeakCheckingStateChanged(bool isChecking)
{
    ui->actionCheckLeaked->setEnabled(!isChecking);
}

// --- PR20: batch check ---
void MainWindow::onCheckAllPasswords()
{
    if (m_batchChecker->isRunning()) return;
    const auto tasks = collectAllTasks();
    if (tasks.isEmpty()) {
        QMessageBox::information(this, tr("Масова перевірка"), tr("Немає записів для перевірки."));
        return;
    }
    m_batchChecker->startCheck(tasks);
}

void MainWindow::onCancelBatchCheck()
{
    if (m_batchChecker->isRunning()) {
        m_batchChecker->cancelCheck();
        statusBar()->showMessage(tr("Скасування…"));
    }
}

void MainWindow::onBatchStarted(int totalCount)
{
    setBatchCheckRunning(true);
    ui->progressBar->setMaximum(totalCount);
    ui->progressBar->setValue(0);
    statusBar()->showMessage(tr("Масова перевірка: 0 / %1…").arg(totalCount));
}

void MainWindow::onBatchProgressChanged(int checked, int total)
{
    // Це вже GUI-потік (QueuedConnection через invokeMethod)
    ui->progressBar->setValue(checked);
    statusBar()->showMessage(tr("Перевірено: %1 / %2…").arg(checked).arg(total));
}

void MainWindow::onBatchSingleResultReady(SingleCheckResult result)
{
    applyLeakStatusToModel(result);
}

void MainWindow::onBatchFinished(BatchCheckResult result)
{
    setBatchCheckRunning(false);
    ui->progressBar->setValue(result.total);

    const QString summary = tr("Завершено.\n\nВсього: %1\nПеревірено: %2\nСкомпрометовано: %3\nПомилок: %4")
        .arg(result.total).arg(result.checked).arg(result.compromised).arg(result.failed);

    result.compromised > 0
        ? QMessageBox::warning(this, tr("Результат"), summary)
        : QMessageBox::information(this, tr("Результат"), summary);

    statusBar()->showMessage(
        tr("Завершено: %1 скомпрометовано з %2").arg(result.compromised).arg(result.total), 8000);
}

void MainWindow::onBatchCancelled()
{
    setBatchCheckRunning(false);
    statusBar()->showMessage(tr("Масову перевірку скасовано."), 5000);
}

// --- Helpers ---
void MainWindow::setBatchCheckRunning(bool running)
{
    ui->actionCheckAllPasswords->setEnabled(!running);
    ui->actionCancelBatchCheck->setVisible(running);
    ui->progressBar->setVisible(running);
    ui->actionCheckLeaked->setEnabled(!running);
}

void MainWindow::applyLeakStatusToModel(const SingleCheckResult &result)
{
    // Оновлюємо модель виключно тут — у GUI-потоці
    // TODO: m_model->setLeakStatus(result.taskId, result.status, result.leakCount);
    Q_UNUSED(result)
}

void MainWindow::updateStatusBar()
{
    statusBar()->showMessage(
        tr("Total: %1  Filtered: %2").arg(m_model->rowCount()).arg(m_proxyModel->rowCount()));
}

QString MainWindow::currentSelectedPassword() const
{
    const auto sel = ui->tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) return {};
    // TODO: повернути реальний пароль із m_model
    return {};
}

QList<CredentialCheckTask> MainWindow::collectAllTasks() const
{
    QList<CredentialCheckTask> tasks;
    // TODO: for (int r = 0; r < m_model->rowCount(); ++r) { ... tasks.append(...); }
    return tasks;
}

void MainWindow::onSearchChanged(const QString &text) { m_proxyModel->setFilterFixedString(text); updateStatusBar(); }
void MainWindow::onClearSearch()                      { ui->searchLineEdit->clear(); }
void MainWindow::onCategoryFilterChanged(int)         { updateStatusBar(); }
