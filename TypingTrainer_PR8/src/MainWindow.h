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
    void onStartTraining();
    void onRestartTraining();
    void onReturnToMain();
    void onActionAbout();
    void onActionSettings();
    void onLessonChanged(int index);
    void onRandomLesson();
    void onReloadLessons();
    void onStepChar();

private:
    void    setupConnections();
    void    scanLessons();
    void    loadLessonFile(const QString &path);
    QString lessonsDir() const;
    void    updateTrainingDisplay();
    void    updateStartButtons();

    Ui::MainWindow *ui;
    TextModel m_model;
};
