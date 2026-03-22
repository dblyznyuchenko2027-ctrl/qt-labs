#pragma once

#include <QMainWindow>
#include <QPointer>
#include "TextModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // eventFilter — intercepts key events on the Training page
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onStartTraining();
    void onRestartTraining();
    void onReturnToMain();
    void onActionAbout();
    void onActionSettings();
    void onLessonChanged(int index);
    void onRandomLesson();
    void onReloadLessons();

private:
    void setupConnections();

    // File-system helpers (PR8)
    void    scanLessons();
    void    loadLessonFile(const QString &path);
    QString lessonsDir() const;

    // Keyboard input handler (PR9)
    void handleKeyPress(QKeyEvent *e);

    // Display updaters (PR9)
    void updateTrainingDisplay();
    void updateStartButtons();

    // Virtual keyboard highlight (PR9)
    void highlightVirtualKey(const QString &ch, Qt::Key key);
    void clearVirtualKeyHighlight();
    QPushButton *findKeyButton(const QString &ch, Qt::Key key) const;

    // Rich-text builder for a line (PR9)
    // Returns HTML with coloured spans: done=blue, error=red, current=yellow, rest=plain
    QString buildLineHtml(int lineIdx, int upToChar) const;

    Ui::MainWindow *ui;
    TextModel m_model;

    // Currently highlighted virtual key button (to restore it)
    QPointer<QPushButton> m_highlightedKey;

    // Track whether training session is active
    bool m_trainingActive = false;
};
