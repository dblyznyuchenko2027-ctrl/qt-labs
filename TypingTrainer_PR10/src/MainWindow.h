#pragma once

#include <QElapsedTimer>
#include <QMainWindow>
#include <QPointer>
#include <QSettings>
#include <QTimer>
#include "TextModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPushButton;

// ── Session statistics ────────────────────────────────────────────────────────
struct SessionStats {
    int  totalKeystrokes = 0;   // every key press (correct + wrong)
    int  correctChars    = 0;   // characters accepted as correct
    int  errors          = 0;   // wrong key presses

    void reset() { totalKeystrokes = correctChars = errors = 0; }

    // Accuracy 0-100 %. Returns 100 when nothing typed yet.
    int accuracy() const {
        if (totalKeystrokes == 0) return 100;
        return qRound(100.0 * correctChars / totalKeystrokes);
    }

    // CPM: characters per minute from elapsed milliseconds
    int cpm(qint64 elapsedMs) const {
        if (elapsedMs < 500) return 0;   // avoid huge numbers at start
        return qRound(correctChars * 60000.0 / elapsedMs);
    }

    // WPM: CPM / 5 (standard convention)
    int wpm(qint64 elapsedMs) const { return cpm(elapsedMs) / 5; }
};

// ── Speed mode ───────────────────────────────────────────────────────────────
enum class SpeedMode { CPM, WPM };

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // Navigation
    void onStartTraining();
    void onRestartTraining();
    void onReturnToMain();

    // Menu
    void onActionAbout();
    void onActionSettings();
    void onActionToggleSpeedMode();

    // Lesson management
    void onLessonChanged(int index);
    void onRandomLesson();
    void onReloadLessons();

    // Timer tick
    void onTimerTick();

private:
    void setupConnections();

    // File-system helpers
    void    scanLessons();
    void    loadLessonFile(const QString &path);
    QString lessonsDir() const;

    // Session control
    void startSession();
    void stopSession();
    void resetSessionCounters();
    void finishSession();           // called when lesson completed

    // Key handling
    void handleKeyPress(QKeyEvent *e);

    // Display
    void updateTrainingDisplay();
    void updateMetricsDisplay();    // refresh Time / Speed / Accuracy labels
    void updateStartButtons();
    void showResults();             // copy metrics to Results page

    // Virtual keyboard
    void highlightVirtualKey(const QString &ch, Qt::Key key);
    void clearVirtualKeyHighlight();
    QPushButton *findKeyButton(const QString &ch, Qt::Key key) const;

    // Rich-text builder
    QString buildLineHtml(int lineIdx, int upToChar) const;

    // QSettings helpers
    void loadSettings();
    void saveSettings();

    // ── Members ──────────────────────────────────────────────────────────────
    Ui::MainWindow *ui;
    TextModel       m_model;

    // Timer & elapsed time
    QTimer        m_timer;
    QElapsedTimer m_elapsed;        // monotonic wall-clock for accurate duration

    // Session state
    SessionStats  m_stats;
    SpeedMode     m_speedMode = SpeedMode::CPM;
    bool          m_trainingActive = false;

    // Virtual key highlight
    QPointer<QPushButton> m_highlightedKey;

    // Settings
    QSettings m_settings;

    // Actions (created in setupMenuBar / setupConnections)
    QAction *m_actToggleSpeed = nullptr;
};
