#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>
#include <QStringList>

// ─── Lesson data ────────────────────────────────────────────────────────────
static const QStringList LESSON_DESCRIPTIONS = {
    "Home row keys: asdfjkl; — the foundation of touch typing.",
    "Practice digits 0-9 and common symbols: ! @ # $ % & * ( )",
    "High-frequency English words using home and top row keys.",
    "Continuation of common words with punctuation and capitals.",
    "Short Python code snippets — great for programmer speed.",
    "Classic Lorem Ipsum paragraph — mixed vocabulary and rhythm."
};

static const QStringList LESSON_TEXTS = {
    "jfj j jff ffjjjj fjjf fjjj jjjjjjj\njfj j jff ffjjjj Fjn ffj",
    "12 34 56 78 90 11 22 33 44 55\n66 77 88 99 00 123 456 789",
    "the and for are but not you all can her\nwas one our out day get has him his how",
    "time two more write go see number no way could\npeople my than first water been call who oil its",
    "def hello():\n    print(\"Hello, world!\")\n\nif __name__ == \"__main__\":\n    hello()",
    "Lorem ipsum dolor sit amet consectetur adipiscing elit\nsed do eiusmod tempor incididunt ut labore"
};

// ─── Constructor ────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Style keyboard buttons uniformly
    const QString keyStyle =
        "QPushButton { border: 1px solid palette(mid); border-radius: 4px; "
        "              padding: 4px 2px; min-height: 28px; background: palette(button); }"
        "QPushButton:hover  { background: palette(light); }"
        "QPushButton[highlighted=true] { background: #4a90d9; color: white; border-color: #2a70b9; }";
    ui->frameKeyboard->setStyleSheet(keyStyle);

    // Style text display area
    ui->textDisplay->setStyleSheet(
        "QTextEdit { background: palette(base); border: none; padding: 8px; line-height: 150%; }");

    setupConnections();
    updateLessonDescription(0);

    // Start on page 0 (Start screen)
    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ─── Connections ────────────────────────────────────────────────────────────
void MainWindow::setupConnections()
{
    // Navigation buttons
    connect(ui->btnStartTraining,   &QPushButton::clicked, this, &MainWindow::onStartTraining);
    connect(ui->btnRestartTraining, &QPushButton::clicked, this, &MainWindow::onRestartTraining);
    connect(ui->btnReturnToMain,    &QPushButton::clicked, this, &MainWindow::onReturnToMain);

    // Menu actions
    connect(ui->actionExit,     &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout,    &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onActionSettings);

    // Lesson picker
    connect(ui->comboLesson, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLessonChanged);
}

// ─── Navigation slots ───────────────────────────────────────────────────────
void MainWindow::onStartTraining()
{
    const int idx = ui->comboLesson->currentIndex();
    if (idx >= 0 && idx < LESSON_TEXTS.size()) {
        ui->textDisplay->setPlainText(LESSON_TEXTS.at(idx));
    }

    // Reset session indicators
    ui->lblTimeValue->setText("00:00");
    ui->lblSpeedValue->setText("0 CPM");
    ui->lblAccuracyValue->setText("100%");
    ui->progressAccuracy->setValue(100);

    ui->stackScreens->setCurrentIndex(1);   // Training page
    ui->textDisplay->setFocus();
}

void MainWindow::onRestartTraining()
{
    onStartTraining();
}

void MainWindow::onReturnToMain()
{
    // Populate results page before switching away (demo values here)
    ui->stackScreens->setCurrentIndex(0);   // Start page
    ui->comboLesson->setFocus();
}

// ─── Menu slots ─────────────────────────────────────────────────────────────
void MainWindow::onActionAbout()
{
    QMessageBox::about(this,
        tr("About TypingTrainer"),
        tr("<b>TypingTrainer</b> v1.0<br><br>"
           "A touch-typing practice application built with Qt Widgets.<br>"
           "Practical work #6 — QMainWindow, QStackedWidget, Layouts."));
}

void MainWindow::onActionSettings()
{
    QMessageBox::information(this,
        tr("Settings"),
        tr("Settings dialog will be implemented in a future practical work."));
}

// ─── Lesson description ─────────────────────────────────────────────────────
void MainWindow::onLessonChanged(int index)
{
    updateLessonDescription(index);
}

void MainWindow::updateLessonDescription(int index)
{
    if (index >= 0 && index < LESSON_DESCRIPTIONS.size()) {
        ui->lblLessonDesc->setText(LESSON_DESCRIPTIONS.at(index));
    }
}
