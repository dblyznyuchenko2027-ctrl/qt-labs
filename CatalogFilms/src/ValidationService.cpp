#include "ValidationService.h"

#include <QObject>

bool ValidationService::validateMovie(const Movie &movie, QString &errorMessage)
{
    if (movie.title.trimmed().isEmpty()) {
        errorMessage = QObject::tr("Назва фільму не може бути порожньою.");
        return false;
    }
    if (movie.year < 1888 || movie.year > 2030) {
        errorMessage = QObject::tr("Рік повинен бути у діапазоні 1888-2030.");
        return false;
    }
    if (movie.rating < 0 || movie.rating > 10) {
        errorMessage = QObject::tr("Оцінка повинна бути від 0 до 10.");
        return false;
    }
    if (movie.status != "Переглянуто" && movie.status != "Хочу подивитися") {
        errorMessage = QObject::tr("Невідомий статус фільму.");
        return false;
    }
    return true;
}
