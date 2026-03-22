#pragma once

#include <QMainWindow>
#include "TextModel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Navigation
    void onStartTraining();
    void onRestartTraining();
    void onReturnToMain();

    // Menu
    void onActionAbout();
    void onActionSettings();

    // Lesson selection
    void onLessonChanged(int index);

    // PR7: test step button
    void onStepChar();

private:
    void setupConnections();

    // Model helpers
    void loadLesson(int index);       // load text from built-in data into m_model
    void updateTrainingDisplay();     // refresh Training UI from m_model

    Ui::MainWindow *ui;

    TextModel m_model;
};
