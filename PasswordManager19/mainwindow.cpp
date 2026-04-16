#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "passwordtablemodel.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}
    , ui{new Ui::MainWindow}
{
    ui->setupUi(this);
    setupModel();
    setupConnections();
    updateStatusBar();

    // Ліниво створюємо checker — один екземпляр на весь час життя вікна
    m_leakChecker = new PasswordLeakChecker(this);

    connect(m_leakChecker, &PasswordLeakChecker::checkCompleted,
            this, &MainWindow::onLeakCheckCompleted);
    connect(m_leakChecker, &PasswordLeakChecker::checkFailed,
            this, &MainWindow::onLeakCheckFailed);
    connect(m_leakChecker, &PasswordLeakChecker::checkingStateChanged,
            this, &MainWindow::onLeakCheckingStateChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void MainWindow::setupModel()
{
    m_model = new PasswordTableModel(this);

    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // шукати по всіх стовпцях

    ui->tableView->setModel(m_proxyModel);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSortingEnabled(true);
}

void MainWindow::setupConnections()
{
    // Toolbar / меню
    connect(ui->actionNewEntry,    &QAction::triggered, this, &MainWindow::onNewEntry);
    connect(ui->actionEditEntry,   &QAction::triggered, this, &MainWindow::onEditEntry);
    connect(ui->actionDeleteEntry, &QAction::triggered, this, &MainWindow::onDeleteEntry);
    connect(ui->actionCopyUsername,&QAction::triggered, this, &MainWindow::onCopyUsername);
    connect(ui->actionCopyPassword,&QAction::triggered, this, &MainWindow::onCopyPassword);

    // Кнопка перевірки витоку — запускається за явним запитом користувача,
    // НЕ на кожне натискання клавіші (вимога документації HIBP)
    connect(ui->actionCheckLeaked, &QAction::triggered,
            this, &MainWindow::onCheckPasswordLeaked);

    // Пошук
    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchChanged);
    connect(ui->clearSearchButton, &QPushButton::clicked,
            this, &MainWindow::onClearSearch);

    // Фільтр категорій
    connect(ui->categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCategoryFilterChanged);
}

// ---------------------------------------------------------------------------
// Entry actions
// ---------------------------------------------------------------------------

void MainWindow::onNewEntry()
{
    // TODO: відкрити діалог створення нового запису
    // Після збереження можна відразу запропонувати перевірку пароля
}

void MainWindow::onEditEntry()
{
    const QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::information(this, tr("Редагування"),
                                 tr("Оберіть запис для редагування."));
        return;
    }
    // TODO: відкрити діалог редагування
}

void MainWindow::onDeleteEntry()
{
    const QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return;

    const int answer = QMessageBox::question(
        this, tr("Видалення"),
        tr("Видалити обраний запис?"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (answer == QMessageBox::Yes) {
        const QModelIndex sourceIndex =
            m_proxyModel->mapToSource(selected.first());
        m_model->removeRow(sourceIndex.row());
        updateStatusBar();
    }
}

void MainWindow::onCopyUsername()
{
    // TODO: скопіювати логін обраного запису в буфер
}

void MainWindow::onCopyPassword()
{
    // TODO: скопіювати пароль обраного запису в буфер
}

// ---------------------------------------------------------------------------
// Password leak check
// ---------------------------------------------------------------------------

void MainWindow::onCheckPasswordLeaked()
{
    const QString password = currentSelectedPassword();

    if (password.isEmpty()) {
        QMessageBox::information(this, tr("Перевірка витоку"),
                                 tr("Оберіть запис із паролем для перевірки."));
        return;
    }

    // Перевіряємо лише після явної команди — жодної перевірки при введенні!
    m_leakChecker->checkPassword(password);
}

void MainWindow::onLeakCheckCompleted(bool found, int count)
{
    if (found) {
        QMessageBox::warning(
            this,
            tr("Пароль знайдено у витоках"),
            tr("⚠️ Цей пароль знайдено у злитих базах даних %n раз(ів).\n\n"
               "Рекомендується негайно змінити пароль на унікальний.", "", count)
        );
    } else {
        QMessageBox::information(
            this,
            tr("Пароль не знайдено"),
            tr("✅ Цей пароль не знайдено у відомих злитих базах.\n\n"
               "Це не гарантує повну безпеку, але є хорошим знаком.")
        );
    }

    statusBar()->showMessage(
        found
            ? tr("Пароль скомпрометовано (%1 збігів)").arg(count)
            : tr("Пароль не знайдено у злитих базах"),
        5000
    );
}

void MainWindow::onLeakCheckFailed(const QString &message)
{
    QMessageBox::warning(
        this,
        tr("Помилка перевірки"),
        tr("Не вдалося перевірити пароль:\n\n%1\n\n"
           "Локальні дані залишаються незмінними.").arg(message)
    );

    statusBar()->showMessage(tr("Перевірку не вдалося завершити"), 5000);
}

void MainWindow::onLeakCheckingStateChanged(bool isChecking)
{
    // Блокуємо кнопку на час запиту, щоб не надсилати дублікати
    ui->actionCheckLeaked->setEnabled(!isChecking);

    if (isChecking) {
        statusBar()->showMessage(tr("Перевірка пароля… зачекайте"));
    }

    // Показуємо/ховаємо індикатор прогресу (якщо є у статус-барі)
    if (ui->progressBar) {
        ui->progressBar->setVisible(isChecking);
    }
}

// ---------------------------------------------------------------------------
// Search & filter
// ---------------------------------------------------------------------------

void MainWindow::onSearchChanged(const QString &text)
{
    m_proxyModel->setFilterFixedString(text);
    updateStatusBar();
}

void MainWindow::onClearSearch()
{
    ui->searchLineEdit->clear();
}

void MainWindow::onCategoryFilterChanged(int index)
{
    Q_UNUSED(index)
    // TODO: фільтрація по категорії
    updateStatusBar();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void MainWindow::updateStatusBar()
{
    const int total    = m_model->rowCount();
    const int filtered = m_proxyModel->rowCount();
    statusBar()->showMessage(
        tr("Total: %1  Filtered: %2").arg(total).arg(filtered)
    );
}

QString MainWindow::currentSelectedPassword() const
{
    const QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) return {};

    // TODO: повернути реальний пароль із моделі для обраного рядка
    // Наприклад:
    // const QModelIndex sourceIndex = m_proxyModel->mapToSource(selected.first());
    // return m_model->passwordAt(sourceIndex.row());

    return {}; // замінити реальною реалізацією
}
