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

// ─────────────────────────────────────────────────────────────────────────────
//  Colour constants for rich-text highlighting
// ─────────────────────────────────────────────────────────────────────────────
static constexpr char CLR_DONE[]    = "#cfe8ff";   // light blue  — correctly typed
static constexpr char CLR_ERROR[]   = "#ffb3b3";   // light red   — error position
static constexpr char CLR_CURRENT[] = "#ffe08a";   // yellow      — current char
static constexpr char CLR_KEY_HI[]  = "#4a90d9";   // blue        — virtual key highlight

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Style keyboard buttons base
    const QString keyStyle =
        "QPushButton { border: 1px solid palette(mid); border-radius: 4px; "
        "              padding: 4px 2px; min-height: 28px; background: palette(button); }"
        "QPushButton:hover { background: palette(light); }";
    ui->frameKeyboard->setStyleSheet(keyStyle);

    // Install event filter on Training page so we capture keys
    // regardless of which child widget holds focus inside that page.
    ui->pageTraining->installEventFilter(this);
    ui->pageTraining->setFocusPolicy(Qt::StrongFocus);

    setupConnections();
    scanLessons();

    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

MainWindow::~MainWindow() { delete ui; }

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

    connect(ui->comboLesson, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLessonChanged);
}

// ─────────────────────────────────────────────────────────────────────────────
//  eventFilter — central keyboard handler for Training screen (PR9)
// ─────────────────────────────────────────────────────────────────────────────
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // Only intercept key presses when training is active
    if (m_trainingActive
        && watched == ui->pageTraining
        && event->type() == QEvent::KeyPress)
    {
        handleKeyPress(static_cast<QKeyEvent *>(event));
        return true;  // consume: don't pass to child widgets
    }
    return QMainWindow::eventFilter(watched, event);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Key press handler
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::handleKeyPress(QKeyEvent *e)
{
    if (m_model.isFinished()) return;

    const Qt::Key key   = static_cast<Qt::Key>(e->key());
    const QString text  = e->text();

    // ── Backspace ────────────────────────────────────────────────────────────
    if (key == Qt::Key_Backspace) {
        m_model.goBack();
        highlightVirtualKey(QString(), Qt::Key_Backspace);
        updateTrainingDisplay();
        return;
    }

    // ── Enter / Return — allow moving to next line explicitly ────────────────
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        // Only act if we are at the end of the current line
        if (m_model.charIndex() >= m_model.currentLine().size()) {
            m_model.advance();
            highlightVirtualKey(QString(), Qt::Key_Return);
            updateTrainingDisplay();
        }
        return;
    }

    // ── Ignore modifier-only and navigation keys ─────────────────────────────
    if (text.isEmpty()) return;
    if (key == Qt::Key_Tab || key == Qt::Key_Escape) return;

    // ── Printable character ───────────────────────────────────────────────────
    const QString expected = m_model.currentChar();

    if (!expected.isEmpty() && text != expected) {
        // Wrong key — mark error at current position, don't advance
        m_model.markError();
        highlightVirtualKey(text, key);
        updateTrainingDisplay();
        return;
    }

    // Correct (or end-of-line space — advance past it)
    highlightVirtualKey(text, key);
    m_model.advance();
    updateTrainingDisplay();

    // Check if lesson finished
    if (m_model.isFinished()) {
        m_trainingActive = false;
        ui->lblCurrentLine->setText(
            QStringLiteral("<span style='color:green; font-weight:bold;'>")
            + tr("Lesson complete! Press Restart or Return.")
            + QStringLiteral("</span>"));
        clearVirtualKeyHighlight();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Training display — rich-text line rendering (PR9)
// ─────────────────────────────────────────────────────────────────────────────

// Build HTML for a completed or partially-done line
QString MainWindow::buildLineHtml(int lineIdx, int upToChar) const
{
    const QString line = m_model.lineAt(lineIdx);
    if (line.isEmpty()) return QString();

    QString html;
    html.reserve(line.size() * 30);

    const int currentLine = m_model.lineIndex();
    const int currentChar = m_model.charIndex();

    for (int i = 0; i < line.size(); ++i) {
        const QString ch = QString(line.at(i)).toHtmlEscaped();

        if (lineIdx < currentLine) {
            // Entire previous line: show as "done" (blue tint, dimmed)
            html += QStringLiteral("<span style='background:") + CLR_DONE
                    + QStringLiteral("; color:#555;'>") + ch + QStringLiteral("</span>");
        } else {
            // Current line
            if (i < currentChar) {
                // Already passed
                if (m_model.hasError(lineIdx, i)) {
                    html += QStringLiteral("<span style='background:") + CLR_ERROR
                            + QStringLiteral(";'>") + ch + QStringLiteral("</span>");
                } else {
                    html += QStringLiteral("<span style='background:") + CLR_DONE
                            + QStringLiteral(";'>") + ch + QStringLiteral("</span>");
                }
            } else if (i == currentChar) {
                // Current target char
                html += QStringLiteral("<span style='background:") + CLR_CURRENT
                        + QStringLiteral("; font-weight:bold;'>") + ch + QStringLiteral("</span>");
            } else {
                // Not yet reached
                html += ch;
            }
        }
    }
    return html;
}

void MainWindow::updateTrainingDisplay()
{
    // Previous line (greyed-out, full done highlighting)
    const int prevIdx = m_model.lineIndex() - 1;
    if (prevIdx >= 0) {
        ui->lblPreviousLine->setText(buildLineHtml(prevIdx, m_model.lineAt(prevIdx).size()));
    } else {
        ui->lblPreviousLine->clear();
    }

    // Current line with colour coding
    if (m_model.isFinished()) {
        // Don't overwrite "Lesson complete!" message set in handleKeyPress
        return;
    }
    ui->lblCurrentLine->setText(buildLineHtml(m_model.lineIndex(), m_model.charIndex()));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Virtual keyboard highlight (PR9)
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
    // 1. Try to find by objectName that encodes the character
    //    objectName convention from UI file: "key_a", "key_space", "key_backspace" etc.
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

    // 2. Fallback: match by service key
    switch (key) {
    case Qt::Key_Backspace: return ui->frameKeyboard->findChild<QPushButton *>("key_backspace");
    case Qt::Key_Return:
    case Qt::Key_Enter:     return ui->frameKeyboard->findChild<QPushButton *>("key_enter");
    case Qt::Key_Shift:     return ui->frameKeyboard->findChild<QPushButton *>("key_lshift");
    case Qt::Key_CapsLock:  return ui->frameKeyboard->findChild<QPushButton *>("key_caps");
    default: break;
    }

    // 3. Last resort: scan all buttons by visible text
    if (!ch.isEmpty()) {
        const QString upper = ch.toUpper();
        for (auto *b : ui->frameKeyboard->findChildren<QPushButton *>()) {
            if (b->text().compare(upper, Qt::CaseInsensitive) == 0)
                return b;
        }
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
    m_trainingActive = true;

    ui->lblTimeValue->setText("00:00");
    ui->lblAccuracyValue->setText("100%");
    ui->progressAccuracy->setValue(100);

    clearVirtualKeyHighlight();
    ui->stackScreens->setCurrentIndex(1);
    updateTrainingDisplay();

    // Give focus to Training page so eventFilter receives key events
    ui->pageTraining->setFocus();
}

void MainWindow::onRestartTraining()
{
    onStartTraining();
}

void MainWindow::onReturnToMain()
{
    m_trainingActive = false;
    clearVirtualKeyHighlight();
    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Lesson management (PR8 carry-over)
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
    const QString previousPath = ui->comboLesson->currentData().toString();
    {
        QSignalBlocker blocker(ui->comboLesson);
        ui->comboLesson->clear();
        const QDir dir(lessonsDir());
        if (dir.exists()) {
            const QStringList files = dir.entryList(
                QStringList{QStringLiteral("*.txt")}, QDir::Files, QDir::Name);
            for (const QString &fileName : files) {
                const QString fullPath = dir.filePath(fileName);
                QString title = QFileInfo(fullPath).baseName();
                title.replace(QLatin1Char('_'), QLatin1Char(' '));
                ui->comboLesson->addItem(title, fullPath);
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
void MainWindow::onActionAbout()
{
    QMessageBox::about(this, tr("About TypingTrainer"),
        tr("<b>TypingTrainer</b> v1.0<br><br>"
           "Practical work #9 — keyPressEvent / eventFilter, "
           "rich-text highlighting, virtual keyboard."));
}

void MainWindow::onActionSettings()
{
    QMessageBox::information(this, tr("Settings"),
        tr("Settings will be implemented in a future practical work.\n\n"
           "Lessons folder:\n%1").arg(lessonsDir()));
}
