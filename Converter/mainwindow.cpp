#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // UI is fully defined in mainwindow.ui (Qt Designer).
    // Conversion logic will be connected in ПР3.
}

MainWindow::~MainWindow()
{
    delete ui;
}
