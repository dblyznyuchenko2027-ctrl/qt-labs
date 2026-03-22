#pragma once

#include <QMainWindow>

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

private:
    void setupConnections();
    void updateLessonDescription(int index);

    Ui::MainWindow *ui;
};
