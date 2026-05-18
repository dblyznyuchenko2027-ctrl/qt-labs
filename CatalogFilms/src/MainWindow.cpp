#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MovieDialog.h"
#include "SettingsWindow.h"
#include "MovieTableModel.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QApplication>

namespace {
int sortColumnFromName(const QString &name)
{
    if (name == "year")   return COL_YEAR;
    if (name == "rating") return COL_RATING;
    if (name == "genre")  return COL_GENRE;
    return COL_TITLE;
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupDatabase();
    setupModel();
    setupConnections();
    reloadData();

    Settings s = m_settingsMgr->currentSettings();
    applyTheme(s.theme);
    m_proxy->sort(sortColumnFromName(s.sortColumn),
                  s.sortOrder == "desc" ? Qt::DescendingOrder : Qt::AscendingOrder);

    updateActions();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete m_movieService;
    delete m_settingsMgr;
    delete m_movieRepo;
    delete m_settingsRepo;
    delete m_dbManager;
    delete ui;
}

void MainWindow::setupDatabase()
{
    // Persist the DB in the user's data directory so future runs see the same catalog.
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + "/movies.db";

    m_dbManager = new DatabaseManager();
    if (!m_dbManager->open(dbPath)) {
        QMessageBox::critical(this, tr("Помилка БД"),
            tr("Не вдалося відкрити базу даних:\n%1").arg(m_dbManager->lastError()));
        return;
    }
    if (!m_dbManager->initializeSchema()) {
        QMessageBox::critical(this, tr("Помилка БД"),
            tr("Не вдалося ініціалізувати схему БД."));
        return;
    }

    m_movieRepo    = new MovieRepository(m_dbManager->database());
    m_settingsRepo = new SettingsRepository(m_dbManager->database());
    m_movieService = new MovieService(m_movieRepo);
    m_settingsMgr  = new SettingsManager(m_settingsRepo);
}

void MainWindow::setupModel()
{
    m_model = new MovieTableModel(this);
    m_proxy = new SearchManager(this);
    m_proxy->setSourceModel(m_model);

    ui->movieTable->setModel(m_proxy);
    ui->movieTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->movieTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->movieTable->setAlternatingRowColors(true);
    ui->movieTable->setSortingEnabled(true);

    ui->movieTable->horizontalHeader()->setSectionResizeMode(COL_TITLE, QHeaderView::Stretch);
    ui->movieTable->horizontalHeader()->setSectionResizeMode(COL_GENRE, QHeaderView::ResizeToContents);
    ui->movieTable->horizontalHeader()->setSectionResizeMode(COL_STATUS, QHeaderView::ResizeToContents);
    ui->movieTable->verticalHeader()->setVisible(false);
}

void MainWindow::setupConnections()
{
    connect(ui->addButton,      &QPushButton::clicked, this, &MainWindow::onAddMovie);
    connect(ui->editButton,     &QPushButton::clicked, this, &MainWindow::onEditMovie);
    connect(ui->deleteButton,   &QPushButton::clicked, this, &MainWindow::onDeleteMovie);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettings);

    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
    connect(ui->statusFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStatusFilterChanged);

    connect(ui->movieTable->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(ui->movieTable, &QTableView::doubleClicked,
            this, &MainWindow::onDoubleClick);
}

void MainWindow::reloadData()
{
    if (!m_movieService) return;
    m_model->setMovies(m_movieService->allMovies());
    updateStatusBar();
    updateActions();
}

int MainWindow::currentSourceRow() const
{
    QModelIndex idx = ui->movieTable->currentIndex();
    if (!idx.isValid()) return -1;
    return m_proxy->mapToSource(idx).row();
}

void MainWindow::onAddMovie()
{
    MovieDialog dialog(this);
    Movie m;
    m.year   = 2024;
    m.status = "Хочу подивитися";
    dialog.setMovie(m);

    if (dialog.exec() != QDialog::Accepted) return;

    Movie newMovie = dialog.movie();
    if (!m_movieService->addMovie(newMovie)) {
        QMessageBox::warning(this, tr("Помилка"),
            tr("Не вдалося додати фільм:\n%1").arg(m_movieService->lastError()));
        return;
    }
    m_model->addMovie(newMovie);
    updateStatusBar();
}

void MainWindow::onEditMovie()
{
    int row = currentSourceRow();
    if (row < 0) return;

    Movie current = m_model->movieAt(row);
    MovieDialog dialog(this);
    dialog.setMovie(current);
    if (dialog.exec() != QDialog::Accepted) return;

    Movie updated = dialog.movie();
    updated.id = current.id;
    if (!m_movieService->updateMovie(updated)) {
        QMessageBox::warning(this, tr("Помилка"),
            tr("Не вдалося оновити фільм:\n%1").arg(m_movieService->lastError()));
        return;
    }
    m_model->updateMovie(row, updated);
    updateStatusBar();
}

void MainWindow::onDeleteMovie()
{
    int row = currentSourceRow();
    if (row < 0) return;

    Movie current = m_model->movieAt(row);
    auto answer = QMessageBox::question(this, tr("Підтвердження"),
        tr("Видалити фільм \"%1\"?").arg(current.title),
        QMessageBox::Yes | QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    if (!m_movieService->removeMovie(current.id)) {
        QMessageBox::warning(this, tr("Помилка"),
            tr("Не вдалося видалити фільм:\n%1").arg(m_movieService->lastError()));
        return;
    }
    m_model->removeMovie(row);
    updateStatusBar();
    updateActions();
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_proxy->setSearchText(text);
    updateStatusBar();
}

void MainWindow::onStatusFilterChanged(int index)
{
    QString status = ui->statusFilterCombo->itemText(index);
    m_proxy->setStatusFilter(status);
    updateStatusBar();
}

void MainWindow::onSelectionChanged()
{
    updateActions();
}

void MainWindow::onDoubleClick(const QModelIndex &index)
{
    Q_UNUSED(index);
    onEditMovie();
}

void MainWindow::onSettings()
{
    SettingsWindow dialog(this);
    dialog.setSettings(m_settingsMgr->currentSettings());
    if (dialog.exec() != QDialog::Accepted) return;

    Settings s = dialog.settings();
    if (!m_settingsMgr->saveSettings(s)) {
        QMessageBox::warning(this, tr("Помилка"),
            tr("Не вдалося зберегти налаштування:\n%1").arg(m_settingsMgr->lastError()));
        return;
    }
    applyTheme(s.theme);
    m_proxy->sort(sortColumnFromName(s.sortColumn),
                  s.sortOrder == "desc" ? Qt::DescendingOrder : Qt::AscendingOrder);
}

void MainWindow::updateActions()
{
    bool hasSelection = currentSourceRow() >= 0;
    ui->editButton->setEnabled(hasSelection);
    ui->deleteButton->setEnabled(hasSelection);
}

void MainWindow::updateStatusBar()
{
    int total    = m_model ? m_model->count() : 0;
    int visible  = m_proxy ? m_proxy->rowCount() : 0;
    statusBar()->showMessage(
        tr("Усього фільмів: %1   |   Відображається: %2").arg(total).arg(visible));
}

void MainWindow::applyTheme(const QString &theme)
{
    if (theme == "dark") {
        qApp->setStyleSheet(
            "QMainWindow, QDialog { background-color: #2b2b2b; color: #e0e0e0; }"
            "QWidget { background-color: #2b2b2b; color: #e0e0e0; }"
            "QLineEdit, QComboBox, QSpinBox, QTextEdit, QTableView {"
            "  background-color: #3c3c3c; color: #e0e0e0; border: 1px solid #555; }"
            "QHeaderView::section { background-color: #444; color: #e0e0e0; }"
            "QPushButton { background-color: #555; color: #e0e0e0;"
            "  border: 1px solid #777; padding: 5px 12px; border-radius: 3px; }"
            "QPushButton:hover { background-color: #666; }"
            "QPushButton:disabled { background-color: #3a3a3a; color: #888; }"
            "QTableView::item:alternate { background-color: #353535; }"
            "QStatusBar { background-color: #2b2b2b; color: #e0e0e0; }"
        );
    } else {
        qApp->setStyleSheet("");
    }
}
