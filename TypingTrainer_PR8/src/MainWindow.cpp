#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QTextStream>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QString keyStyle =
        "QPushButton { border: 1px solid palette(mid); border-radius: 4px; "
        "padding: 4px 2px; min-height: 28px; background: palette(button); }"
        "QPushButton:hover { background: palette(light); }";
    ui->frameKeyboard->setStyleSheet(keyStyle);

    setupConnections();
    scanLessons();
    ui->stackScreens->setCurrentIndex(0);
    ui->comboLesson->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->btnStartTraining,   &QPushButton::clicked, this, &MainWindow::onStartTraining);
    connect(ui->btnRestartTraining, &QPushButton::clicked, this, &MainWindow::onRestartTraining);
    connect(ui->btnReturnToMain,    &QPushButton::clicked, this, &MainWindow::onReturnToMain);
    connect(ui->btnRandomLesson,    &QPushButton::clicked, this, &MainWindow::onRandomLesson);
    connect(ui->btnReloadLessons,   &QPushButton::clicked, this, &MainWindow::onReloadLessons);
    connect(ui->btnStepChar,        &QPushButton::clicked, this, &MainWindow::onStepChar);
    connect(ui->actionExit,         &QAction::triggered,   this, &QWidget::close);
    connect(ui->actionAbout,        &QAction::triggered,   this, &MainWindow::onActionAbout);
    connect(ui->actionSettings,     &QAction::triggered,   this, &MainWindow::onActionSettings);
    connect(ui->comboLesson, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLessonChanged);
}

// ── File-system helpers ───────────────────────────────────────────────────────

QString MainWindow::lessonsDir() const
{
    // Primary: lessons/ next to executable
    QString dir = QCoreApplication::applicationDirPath() + QStringLiteral("/lessons");
    if (!QDir(dir).exists())
        dir = QDir::currentPath() + QStringLiteral("/lessons");
    return dir;
}

void MainWindow::scanLessons()
{
    // Remember current file path to restore selection after reload
    const QString previousPath = ui->comboLesson->currentData().toString();

    {
        QSignalBlocker blocker(ui->comboLesson);
        ui->comboLesson->clear();

        const QDir dir(lessonsDir());
        if (!dir.exists()) {
            qWarning() << "[scanLessons] Directory not found:" << dir.absolutePath();
            // Leave combo empty — no crash
        } else {
            const QStringList files = dir.entryList(
                QStringList{QStringLiteral("*.txt")},
                QDir::Files,
                QDir::Name);

            if (files.isEmpty()) {
                qWarning() << "[scanLessons] No .txt files in:" << dir.absolutePath();
            } else {
                for (const QString &fileName : files) {
                    const QString fullPath = dir.filePath(fileName);
                    const QFileInfo fi(fullPath);
                    QString title = fi.baseName();
                    title.replace(QLatin1Char('_'), QLatin1Char(' '));
                    ui->comboLesson->addItem(title, fullPath);
                }
                qDebug() << "[scanLessons] Found" << files.size() << "lesson(s) in" << dir.absolutePath();
            }
        }
    }

    // Restore previous selection if still present, else pick index 0
    int restoreIndex = 0;
    for (int i = 0; i < ui->comboLesson->count(); ++i) {
        if (ui->comboLesson->itemData(i).toString() == previousPath) {
            restoreIndex = i;
            break;
        }
    }

    // Setting index triggers onLessonChanged -> loadLessonFile
    if (ui->comboLesson->count() > 0)
        ui->comboLesson->setCurrentIndex(restoreIndex);

    const QString msg = tr("Found %1 lesson(s)").arg(ui->comboLesson->count());
    statusBar()->showMessage(msg, 4000);
    updateStartButtons();
}

void MainWindow::loadLessonFile(const QString &path)
{
    if (path.isEmpty()) {
        m_model.setText(QString());
        ui->lblLessonDesc->setText(tr("No lesson selected."));
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString err = tr("Cannot open file:\n%1\n\n%2").arg(path, file.errorString());
        qWarning() << "[loadLessonFile]" << err;
        QMessageBox::warning(this, tr("File Error"), err);
        m_model.setText(QString());
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString text = in.readAll();

    // Normalize line endings (\r\n and \r -> \n)
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    m_model.setText(text);

    const QFileInfo fi(path);
    ui->lblLessonDesc->setText(
        tr("File: %1  |  Size: %2 bytes  |  Lines: %3")
        .arg(fi.fileName())
        .arg(QString::number(fi.size()))
        .arg(QString::number(m_model.lineCount()))
    );

    qDebug() << "[loadLessonFile]" << fi.fileName()
             << "lines:" << m_model.lineCount();
}

// ── UI helpers ───────────────────────────────────────────────────────────────

void MainWindow::updateStartButtons()
{
    const bool has = ui->comboLesson->count() > 0;
    ui->btnStartTraining->setEnabled(has);
    ui->btnRandomLesson->setEnabled(has);
}

void MainWindow::updateTrainingDisplay()
{
    ui->lblPreviousLine->setText(m_model.previousLine());

    QString display;
    if (m_model.isFinished()) {
        display = tr("[Finished — press Restart]");
    } else {
        const QString done   = m_model.donePart();
        const QString cur    = m_model.currentChar();
        const QString remain = m_model.remainPart();
        display = done
                  + (cur.isEmpty() ? QStringLiteral("▌")
                                   : QStringLiteral("[") + cur + QStringLiteral("]"))
                  + remain;
    }
    ui->lblCurrentLine->setText(display);

    ui->lblSpeedValue->setText(
        tr("L%1/%2 C%3/%4")
        .arg(m_model.lineIndex() + 1).arg(m_model.lineCount())
        .arg(m_model.charIndex()).arg(m_model.currentLine().size())
    );
}

// ── Navigation slots ─────────────────────────────────────────────────────────

void MainWindow::onStartTraining()
{
    if (ui->comboLesson->count() == 0) return;
    m_model.resetPosition();
    ui->lblTimeValue->setText("00:00");
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

// ── Lesson management slots ───────────────────────────────────────────────────

void MainWindow::onLessonChanged(int index)
{
    if (index < 0 || index >= ui->comboLesson->count()) return;
    const QString path = ui->comboLesson->itemData(index).toString();
    loadLessonFile(path);
    if (ui->stackScreens->currentIndex() == 1)
        updateTrainingDisplay();
}

void MainWindow::onRandomLesson()
{
    const int n = ui->comboLesson->count();
    if (n <= 0) return;  // edge case: no lessons loaded
    const int idx = static_cast<int>(
        QRandomGenerator::global()->bounded(static_cast<quint32>(n)));
    ui->comboLesson->setCurrentIndex(idx);  // triggers onLessonChanged
    statusBar()->showMessage(
        tr("Random: %1").arg(ui->comboLesson->currentText()), 3000);
}

void MainWindow::onReloadLessons()
{
    scanLessons();
}

// ── Step test slot ────────────────────────────────────────────────────────────

void MainWindow::onStepChar()
{
    if (m_model.isFinished()) {
        ui->lblCurrentLine->setText(tr("[Text finished — use Restart]"));
        return;
    }
    m_model.advance();
    updateTrainingDisplay();
}

// ── Menu slots ────────────────────────────────────────────────────────────────

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, tr("About TypingTrainer"),
        tr("<b>TypingTrainer</b> v1.0<br><br>"
           "Practical work #8 — QDir, QFile, QTextStream, QRandomGenerator.<br>"
           "Lessons folder: <tt>lessons/</tt> next to the executable."));
}

void MainWindow::onActionSettings()
{
    QMessageBox::information(this, tr("Settings"),
        tr("Settings will be implemented in a future practical work.\n\n"
           "Lessons folder:\n%1").arg(lessonsDir()));
}
