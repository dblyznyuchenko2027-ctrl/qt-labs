#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // QSettings uses these to locate the ini/registry key
    app.setOrganizationName("QtPractice");
    app.setOrganizationDomain("qtpractice.local");
    app.setApplicationName("TypingTrainer");
    app.setApplicationVersion("1.0");

    MainWindow window;
    window.show();

    return app.exec();
}
