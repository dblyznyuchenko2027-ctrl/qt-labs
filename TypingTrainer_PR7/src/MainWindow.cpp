#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QMessageBox>

// ─── Built-in lesson data ────────────────────────────────────────────────────
struct LessonData {
    QString name;
    QString text;
    QString description;
};

static const QList<LessonData> LESSONS = {
    {
        "Starter Text",
        "jfj j jff ffjjjj fjjf fjjj jjjjjjj\n"
        "jfj j jff ffjjjj fjn ffj\n"
        "fff jjj fjf jfj fff jjj",
        "Home row keys: asdfjkl; — the foundation of touch typing."
    },
    {
        "Numbers and Symbols",
        "12 34 56 78 90 11 22 33\n"
        "44 55 66 77 88 99 00 123\n"
        "456 789 !@# $%^ &*() -=+",
        "Practice digits 0-9 and common symbols."
    },
    {
        "Common Words Part 1",
        "the and for are but not you all\n"
        "can her was one our out day get\n"
        "has him his how man new now old",
        "High-frequency English words using home and top row keys."
    },
    {
        "Common Words Part 2",
        "time two more write go see number\n"
        "no way could people my than first\n"
        "water been call who oil its long",
        "Continuation of common words with punctuation and capitals."
    },
    {
        "Python Code Sample",
        "def hello():\n"
        "    print(\"Hello, world!\")\n"
        "\n"
        "if __name__ == \"__main__\":\n"
        "    hello()",
        "Short Python code snippets — great for programmer speed."
    },
    {
        "Lorem Ipsum",
        "Lorem ipsum dolor sit amet consectetur\n"
        "adipiscing elit sed do eiusmod tempor\n"
        "incididunt ut labore et dolore magna aliqua",
        "Classic Lorem Ipsum paragraph — mixed vocabulary and rhythm."
    }
};

// ─── Constructor ────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Populate combo box
    for (const auto &lesson : LESSONS)
        ui->comboLesson->addItem(lesson.name);

    // Style keyboard
    const QString keyStyle =
        "QPushButton { border: 1px solid palette(mid); border-radius: 4px; "
        "padding: 4px 2px; min-height: 28px; background: palette(button); }"
        "QPushButton:hover { background: palette(light); }";
    ui->frameKeyboard->setStyleSheet(keyStyle);

    setupConnections();

    // Load first lesson into model
    loadLesson(0);
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
    connect(ui->btnStartTraining,   &QPushButton::clicked, this, &MainWindow::onStartTraining);
    connect(ui->btnRestartTraining, &QPushButton::clicked, this, &MainWindow::onRestartTraining);
    connect(ui->btnReturnToMain,    &QPushButton::clicked, this, &MainWindow::onReturnToMain);
    connect(ui->btnStepChar,        &QPushButton::clicked, this, &MainWindow::onStepChar);

    connect(ui->actionExit,     &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout,    &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onActionSettings);

    connect(ui->comboLesson, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLessonChanged);
}

// ─── Model helpers ───────────────────────────────────────────────────────────
void MainWindow::loadLesson(int index)
{
    if (index < 0 || index >= LESSONS.size()) return;

    const LessonData &lesson = LESSONS.at(index);
    m_model.setText(lesson.text);

    // Update description label on Start screen
    ui->lblLessonDesc->setText(lesson.description);
}

void MainWindow::updateTrainingDisplay()
{
    // Previous line (greyed)
    ui->lblPreviousLine->setText(m_model.previousLine());

    // Current line: done | currentChar | remain
    // Build a simple plain-text representation for now
    // (rich text / HTML highlighting will be added in a later PR)
    const QString done    = m_model.donePart();
    const QString cur     = m_model.currentChar();
    const QString remain  = m_model.remainPart();

    // Compose with a visible marker around current char
    QString display;
    if (m_model.isFinished()) {
        display = tr("[Finished]");
    } else {
        // Bracket notation: done[X]remain
        display = done + (cur.isEmpty() ? QStringLiteral("▌") : QStringLiteral("[") + cur + QStringLiteral("]")) + remain;
    }
    ui->lblCurrentLine->setText(display);

    // Update position info in status bar area
    const QString pos = tr("Line %1/%2  Char %3/%4")
        .arg(m_model.lineIndex() + 1)
        .arg(m_model.lineCount())
        .arg(m_model.charIndex())
        .arg(m_model.currentLine().size());
    ui->lblSpeedValue->setText(pos);   // reuse speed label temporarily
}

// ─── Navigation slots ───────────────────────────────────────────────────────
void MainWindow::onStartTraining()
{
    // Reset model position (lesson already loaded via onLessonChanged)
    m_model.resetPosition();

    // Reset session indicators
    ui->lblTimeValue->setText("00:00");
    ui->lblSpeedValue->setText("0 CPM");
    ui->lblAccuracyValue->setText("100%");
    ui->progressAccuracy->setValue(100);

    ui->stackScreens->setCurrentIndex(1);
    updateTrainingDisplay();
    ui->btnStepChar->setFocus();
}

void MainWindow::onRestartTraining()
{
    onStartTraining();
}

void MainWindow::onReturnToMain()
{
    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

// ─── PR7 test: step one character ───────────────────────────────────────────
void MainWindow::onStepChar()
{
    if (m_model.isFinished()) {
        ui->lblCurrentLine->setText(tr("[Text finished — use Restart]"));
        return;
    }
    m_model.advance();
    updateTrainingDisplay();
}

// ─── Lesson selection ────────────────────────────────────────────────────────
void MainWindow::onLessonChanged(int index)
{
    loadLesson(index);
    // Also update training display if we happen to be on training screen
    if (ui->stackScreens->currentIndex() == 1)
        updateTrainingDisplay();
}

// ─── Menu slots ─────────────────────────────────────────────────────────────
void MainWindow::onActionAbout()
{
    QMessageBox::about(this,
        tr("About TypingTrainer"),
        tr("<b>TypingTrainer</b> v1.0<br><br>"
           "Practical work #7 — Qt Core strings, TextModel, position tracking."));
}

void MainWindow::onActionSettings()
{
    QMessageBox::information(this, tr("Settings"),
        tr("Settings will be implemented in a future practical work."));
}
