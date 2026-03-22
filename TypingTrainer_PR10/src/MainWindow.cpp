#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTextStream>
#include <QDebug>

// ── Colour constants ──────────────────────────────────────────────────────────
static constexpr char CLR_DONE[]    = "#cfe8ff";
static constexpr char CLR_ERROR[]   = "#ffb3b3";
static constexpr char CLR_CURRENT[] = "#ffe08a";
static constexpr char CLR_KEY_HI[]  = "#4a90d9";

// ── Settings keys ─────────────────────────────────────────────────────────────
static constexpr char SK_LESSON[]     = "session/lastLessonPath";
static constexpr char SK_SPEEDMODE[]  = "session/speedMode";

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_settings(QCoreApplication::organizationName(),
                 QCoreApplication::applicationName())
{
    ui->setupUi(this);

    // Add "Toggle CPM/WPM" to Settings menu
    m_actToggleSpeed = ui->menuSettings->addAction(tr("Switch CPM ↔ WPM"));
    m_actToggleSpeed->setStatusTip(tr("Toggle speed display between CPM and WPM"));

    // Keyboard frame style
    const QString keyStyle =
        "QPushButton { border: 1px solid palette(mid); border-radius: 4px; "
        "              padding: 4px 2px; min-height: 28px; background: palette(button); }"
        "QPushButton:hover { background: palette(light); }";
    ui->frameKeyboard->setStyleSheet(keyStyle);

    // EventFilter on Training page
    ui->pageTraining->installEventFilter(this);
    ui->pageTraining->setFocusPolicy(Qt::StrongFocus);

    // Timer setup: 1-second tick
    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);

    setupConnections();
    loadSettings();    // restore last lesson + speed mode
    scanLessons();     // populate combo (restores saved selection)

    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Connections
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupConnections()
{
    connect(ui->btnStartTraining,   &QPushButton::clicked, this, &MainWindow::onStartTraining);
    connect(ui->btnRestartTraining, &QPushButton::clicked, this, &MainWindow::onRestartTraining);
    connect(ui->btnReturnToMain,    &QPushButton::clicked, this, &MainWindow::onReturnToMain);
    connect(ui->btnRandomLesson,    &QPushButton::clicked, this, &MainWindow::onRandomLesson);
    connect(ui->btnReloadLessons,   &QPushButton::clicked, this, &MainWindow::onReloadLessons);

    connect(ui->actionExit,     &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout,    &QAction::triggered, this, &MainWindow::onActionAbout);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::onActionSettings);
    connect(m_actToggleSpeed,   &QAction::triggered, this, &MainWindow::onActionToggleSpeedMode);

    connect(ui->comboLesson, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLessonChanged);

    // Timer tick → update UI
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
}

// ─────────────────────────────────────────────────────────────────────────────
//  QSettings
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::loadSettings()
{
    // Speed mode
    const QString mode = m_settings.value(SK_SPEEDMODE, QStringLiteral("CPM")).toString();
    m_speedMode = (mode == QStringLiteral("WPM")) ? SpeedMode::WPM : SpeedMode::CPM;
    m_actToggleSpeed->setText(
        m_speedMode == SpeedMode::CPM ? tr("Switch to WPM") : tr("Switch to CPM"));
}

void MainWindow::saveSettings()
{
    // Last lesson path
    const QString path = ui->comboLesson->currentData().toString();
    if (!path.isEmpty())
        m_settings.setValue(SK_LESSON, path);

    // Speed mode
    m_settings.setValue(SK_SPEEDMODE,
        m_speedMode == SpeedMode::CPM ? QStringLiteral("CPM") : QStringLiteral("WPM"));

    m_settings.sync();
}

// ─────────────────────────────────────────────────────────────────────────────
//  eventFilter
// ─────────────────────────────────────────────────────────────────────────────
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (m_trainingActive
        && watched == ui->pageTraining
        && event->type() == QEvent::KeyPress)
    {
        handleKeyPress(static_cast<QKeyEvent *>(event));
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Session control
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::resetSessionCounters()
{
    m_stats.reset();
    m_elapsed.invalidate();
}

void MainWindow::startSession()
{
    resetSessionCounters();
    m_trainingActive = true;
    m_elapsed.start();
    m_timer.start();
    updateMetricsDisplay();
}

void MainWindow::stopSession()
{
    m_timer.stop();
    m_trainingActive = false;
    clearVirtualKeyHighlight();
}

void MainWindow::finishSession()
{
    stopSession();
    showResults();
    ui->stackScreens->setCurrentIndex(2);  // Results page
}

void MainWindow::showResults()
{
    const qint64 ms = m_elapsed.isValid() ? m_elapsed.elapsed() : 0;
    const int secs  = static_cast<int>(ms / 1000);
    const int mins  = secs / 60;
    const int sec2  = secs % 60;

    ui->lblTimeResult->setText(
        QString("%1:%2").arg(mins, 2, 10, QLatin1Char('0'))
                        .arg(sec2, 2, 10, QLatin1Char('0')));

    const int speed = (m_speedMode == SpeedMode::CPM)
                      ? m_stats.cpm(ms) : m_stats.wpm(ms);
    const QString unit = (m_speedMode == SpeedMode::CPM) ? "CPM" : "WPM";
    ui->lblSpeedResult->setText(QString::number(speed) + " " + unit);

    ui->lblAccuracyResult->setText(QString::number(m_stats.accuracy()) + "%");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Timer tick — called every second
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onTimerTick()
{
    updateMetricsDisplay();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Metrics display
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::updateMetricsDisplay()
{
    const qint64 ms   = m_elapsed.isValid() ? m_elapsed.elapsed() : 0;
    const int    secs = static_cast<int>(ms / 1000);
    const int    mins = secs / 60;
    const int    sec2 = secs % 60;

    // Time label: mm:ss
    ui->lblTimeValue->setText(
        QString("%1:%2").arg(mins, 2, 10, QLatin1Char('0'))
                        .arg(sec2, 2, 10, QLatin1Char('0')));

    // Speed label: CPM or WPM
    const int speed = (m_speedMode == SpeedMode::CPM)
                      ? m_stats.cpm(ms) : m_stats.wpm(ms);
    const QString unit = (m_speedMode == SpeedMode::CPM) ? "CPM" : "WPM";
    ui->lblSpeedValue->setText(QString::number(speed) + " " + unit);

    // Accuracy label + progress bar
    const int acc = m_stats.accuracy();
    ui->lblAccuracyValue->setText(QString::number(acc) + "%");
    ui->progressAccuracy->setValue(acc);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Key handler
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::handleKeyPress(QKeyEvent *e)
{
    if (m_model.isFinished()) return;

    const Qt::Key key  = static_cast<Qt::Key>(e->key());
    const QString text = e->text();

    // Backspace
    if (key == Qt::Key_Backspace) {
        m_model.goBack();
        highlightVirtualKey(QString(), Qt::Key_Backspace);
        updateTrainingDisplay();
        return;
    }

    // Enter at end-of-line
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        if (m_model.charIndex() >= m_model.currentLine().size()) {
            m_model.advance();
            highlightVirtualKey(QString(), Qt::Key_Return);
            updateTrainingDisplay();
        }
        return;
    }

    // Ignore non-printable
    if (text.isEmpty()) return;
    if (key == Qt::Key_Tab || key == Qt::Key_Escape) return;

    // Printable character
    ++m_stats.totalKeystrokes;
    const QString expected = m_model.currentChar();

    if (!expected.isEmpty() && text != expected) {
        // Wrong key
        ++m_stats.errors;
        m_model.markError();
        highlightVirtualKey(text, key);
        updateTrainingDisplay();
        updateMetricsDisplay();
        return;
    }

    // Correct
    ++m_stats.correctChars;
    highlightVirtualKey(text, key);
    m_model.advance();
    updateTrainingDisplay();
    updateMetricsDisplay();

    // Check completion
    if (m_model.isFinished()) {
        finishSession();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Training display (rich-text, unchanged from PR9)
// ─────────────────────────────────────────────────────────────────────────────
QString MainWindow::buildLineHtml(int lineIdx, int /*upToChar*/) const
{
    const QString line = m_model.lineAt(lineIdx);
    if (line.isEmpty()) return QString();

    QString html;
    html.reserve(line.size() * 30);
    const int curLine = m_model.lineIndex();
    const int curChar = m_model.charIndex();

    for (int i = 0; i < line.size(); ++i) {
        const QString ch = QString(line.at(i)).toHtmlEscaped();
        if (lineIdx < curLine) {
            html += QStringLiteral("<span style='background:") + CLR_DONE
                    + QStringLiteral("; color:#555;'>") + ch + QStringLiteral("</span>");
        } else {
            if (i < curChar) {
                const char *clr = m_model.hasError(lineIdx, i) ? CLR_ERROR : CLR_DONE;
                html += QStringLiteral("<span style='background:") + clr
                        + QStringLiteral(";'>") + ch + QStringLiteral("</span>");
            } else if (i == curChar) {
                html += QStringLiteral("<span style='background:") + CLR_CURRENT
                        + QStringLiteral("; font-weight:bold;'>") + ch + QStringLiteral("</span>");
            } else {
                html += ch;
            }
        }
    }
    return html;
}

void MainWindow::updateTrainingDisplay()
{
    const int prevIdx = m_model.lineIndex() - 1;
    ui->lblPreviousLine->setText(prevIdx >= 0 ? buildLineHtml(prevIdx, 0) : QString());

    if (!m_model.isFinished())
        ui->lblCurrentLine->setText(buildLineHtml(m_model.lineIndex(), m_model.charIndex()));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Virtual keyboard
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::clearVirtualKeyHighlight()
{
    if (m_highlightedKey) {
        m_highlightedKey->setStyleSheet(QString());
        m_highlightedKey = nullptr;
    }
}

QPushButton *MainWindow::findKeyButton(const QString &ch, Qt::Key key) const
{
    QString name;
    if (!ch.isEmpty()) {
        QChar c = ch.at(0).toLower();
        if (c == QLatin1Char(' '))       name = QStringLiteral("key_space");
        else if (c == QLatin1Char('\t')) name = QStringLiteral("key_tab");
        else                             name = QStringLiteral("key_") + c;
    }
    if (!name.isEmpty()) {
        auto *btn = ui->frameKeyboard->findChild<QPushButton *>(name);
        if (btn) return btn;
    }
    switch (key) {
    case Qt::Key_Backspace: return ui->frameKeyboard->findChild<QPushButton *>("key_backspace");
    case Qt::Key_Return:
    case Qt::Key_Enter:     return ui->frameKeyboard->findChild<QPushButton *>("key_enter");
    default: break;
    }
    if (!ch.isEmpty()) {
        const QString upper = ch.toUpper();
        for (auto *b : ui->frameKeyboard->findChildren<QPushButton *>())
            if (b->text().compare(upper, Qt::CaseInsensitive) == 0) return b;
    }
    return nullptr;
}

void MainWindow::highlightVirtualKey(const QString &ch, Qt::Key key)
{
    clearVirtualKeyHighlight();
    QPushButton *btn = findKeyButton(ch, key);
    if (!btn) return;
    btn->setStyleSheet(
        QStringLiteral("QPushButton { background: ") + CLR_KEY_HI
        + QStringLiteral("; color: white; border: 1px solid #2a70b9; border-radius: 4px; "
                         "padding: 4px 2px; min-height: 28px; font-weight: bold; }"));
    m_highlightedKey = btn;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Navigation slots
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onStartTraining()
{
    if (ui->comboLesson->count() == 0) return;
    m_model.resetPosition();
    clearVirtualKeyHighlight();
    startSession();
    ui->stackScreens->setCurrentIndex(1);
    updateTrainingDisplay();
    ui->pageTraining->setFocus();
    saveSettings();  // save chosen lesson
}

void MainWindow::onRestartTraining()
{
    m_model.resetPosition();
    clearVirtualKeyHighlight();
    startSession();
    updateTrainingDisplay();
    ui->pageTraining->setFocus();
}

void MainWindow::onReturnToMain()
{
    stopSession();
    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

// ─────────────────────────────────────────────────────────────────────────────
//  File-system helpers (PR8 / PR9 carry-over)
// ─────────────────────────────────────────────────────────────────────────────
QString MainWindow::lessonsDir() const
{
    QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/lessons");
    if (!QDir(dir).exists())
        dir = QDir::currentPath() + QStringLiteral("/lessons");
    return dir;
}

void MainWindow::scanLessons()
{
    // Try to restore saved lesson path
    const QString savedPath = m_settings.value(SK_LESSON).toString();
    const QString previousPath = !savedPath.isEmpty()
                                 ? savedPath
                                 : ui->comboLesson->currentData().toString();

    {
        QSignalBlocker blocker(ui->comboLesson);
        ui->comboLesson->clear();
        const QDir dir(lessonsDir());
        if (dir.exists()) {
            const QStringList files = dir.entryList(
                QStringList{QStringLiteral("*.txt")}, QDir::Files, QDir::Name);
            for (const QString &fn : files) {
                const QString fp = dir.filePath(fn);
                QString title = QFileInfo(fp).baseName();
                title.replace(QLatin1Char('_'), QLatin1Char(' '));
                ui->comboLesson->addItem(title, fp);
            }
        }
    }

    int restoreIndex = 0;
    for (int i = 0; i < ui->comboLesson->count(); ++i) {
        if (ui->comboLesson->itemData(i).toString() == previousPath) {
            restoreIndex = i;
            break;
        }
    }
    if (ui->comboLesson->count() > 0)
        ui->comboLesson->setCurrentIndex(restoreIndex);

    statusBar()->showMessage(tr("Found %1 lesson(s)").arg(ui->comboLesson->count()), 4000);
    updateStartButtons();
}

void MainWindow::loadLessonFile(const QString &path)
{
    if (path.isEmpty()) { m_model.setText(QString()); return; }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("File Error"),
            tr("Cannot open:\n%1\n\n%2").arg(path, file.errorString()));
        m_model.setText(QString());
        return;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString text = in.readAll();
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    m_model.setText(text);
    const QFileInfo fi(path);
    ui->lblLessonDesc->setText(
        tr("File: %1  |  Size: %2 bytes  |  Lines: %3")
        .arg(fi.fileName()).arg(fi.size()).arg(m_model.lineCount()));
}

void MainWindow::updateStartButtons()
{
    const bool has = ui->comboLesson->count() > 0;
    ui->btnStartTraining->setEnabled(has);
    ui->btnRandomLesson->setEnabled(has);
}

void MainWindow::onLessonChanged(int index)
{
    if (index < 0 || index >= ui->comboLesson->count()) return;
    loadLessonFile(ui->comboLesson->itemData(index).toString());
    if (ui->stackScreens->currentIndex() == 1)
        updateTrainingDisplay();
}

void MainWindow::onRandomLesson()
{
    const int n = ui->comboLesson->count();
    if (n <= 0) return;
    ui->comboLesson->setCurrentIndex(
        static_cast<int>(QRandomGenerator::global()->bounded(static_cast<quint32>(n))));
    statusBar()->showMessage(tr("Random: %1").arg(ui->comboLesson->currentText()), 3000);
}

void MainWindow::onReloadLessons() { scanLessons(); }

// ─────────────────────────────────────────────────────────────────────────────
//  Menu slots
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onActionToggleSpeedMode()
{
    m_speedMode = (m_speedMode == SpeedMode::CPM) ? SpeedMode::WPM : SpeedMode::CPM;
    m_actToggleSpeed->setText(
        m_speedMode == SpeedMode::CPM ? tr("Switch to WPM") : tr("Switch to CPM"));
    updateMetricsDisplay();
    saveSettings();
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, tr("About TypingTrainer"),
        tr("<b>TypingTrainer</b> v1.0<br><br>"
           "Practical work #10 — QTimer, QElapsedTimer, "
           "CPM/WPM metrics, accuracy, QSettings."));
}

void MainWindow::onActionSettings()
{
    QMessageBox::information(this, tr("Settings"),
        tr("Lessons folder:\n%1\n\nSpeed mode: %2\n\n"
           "Use Settings → Switch CPM/WPM to toggle.")
        .arg(lessonsDir())
        .arg(m_speedMode == SpeedMode::CPM ? "CPM" : "WPM"));
}
