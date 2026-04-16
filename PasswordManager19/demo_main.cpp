#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include "passwordleakchecker.h"

/**
 * Мінімальний демонстраційний застосунок для тестування PasswordLeakChecker
 * без повного UI. Запускається як консольна програма.
 *
 * Збірка (окремо від основного проєкту):
 *   qt-cmake -S . -B build-demo
 *   cmake --build build-demo
 */

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    PasswordLeakChecker checker;

    // Підключаємо сигнали до виводу в консоль
    QObject::connect(&checker, &PasswordLeakChecker::checkCompleted,
                     [](bool found, int count) {
        if (found) {
            qInfo() << "[RESULT] Пароль знайдено у злитих базах!" << count << "збігів.";
        } else {
            qInfo() << "[RESULT] Пароль НЕ знайдено у злитих базах.";
        }
        QCoreApplication::quit();
    });

    QObject::connect(&checker, &PasswordLeakChecker::checkFailed,
                     [](const QString &msg) {
        qWarning() << "[ERROR]" << msg;
        QCoreApplication::quit();
    });

    QObject::connect(&checker, &PasswordLeakChecker::checkingStateChanged,
                     [](bool checking) {
        qDebug() << (checking ? "[INFO] Запит надіслано, очікуємо відповідь..."
                              : "[INFO] Запит завершено.");
    });

    // Тест 1: відомо скомпрометований пароль (password123)
    // Тест 2: унікальний пароль з низькою імовірністю збігу
    const QString testPassword = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : "password123";

    qInfo() << "[INFO] Перевіряємо пароль (через k-anonymity, сам пароль не передається)...";
    checker.checkPassword(testPassword);

    // Завершуємо застосунок якщо нічого не прийшло за 15 с
    QTimer::singleShot(15000, &app, []() {
        qWarning() << "[TIMEOUT] Застосунок завершено за таймаутом.";
        QCoreApplication::quit();
    });

    return app.exec();
}
