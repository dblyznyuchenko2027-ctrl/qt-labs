#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PasswordManager");
    app.setOrganizationName("PracticalWork16");

    MainWindow w;
    w.show();

    return app.exec();
}
