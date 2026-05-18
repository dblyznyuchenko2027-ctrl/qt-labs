#include "SettingsRepository.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

SettingsRepository::SettingsRepository(const QSqlDatabase &db)
    : m_db(db)
{}

Settings SettingsRepository::load() const
{
    Settings s;
    QSqlQuery query(m_db);
    if (!query.exec("SELECT id, theme, sort_column, sort_order, movies_per_page "
                    "FROM settings WHERE id = 1")) {
        m_lastError = query.lastError().text();
        qDebug() << "SettingsRepository::load error:" << m_lastError;
        return s;
    }
    if (query.next()) {
        s.id            = query.value(0).toInt();
        s.theme         = query.value(1).toString();
        s.sortColumn    = query.value(2).toString();
        s.sortOrder     = query.value(3).toString();
        s.moviesPerPage = query.value(4).toInt();
    }
    return s;
}

bool SettingsRepository::save(const Settings &settings)
{
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE settings SET "
        "  theme           = :theme,"
        "  sort_column     = :sortColumn,"
        "  sort_order      = :sortOrder,"
        "  movies_per_page = :perPage "
        "WHERE id = 1"
    );
    query.bindValue(":theme",      settings.theme);
    query.bindValue(":sortColumn", settings.sortColumn);
    query.bindValue(":sortOrder",  settings.sortOrder);
    query.bindValue(":perPage",    settings.moviesPerPage);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "SettingsRepository::save error:" << m_lastError;
        return false;
    }
    return true;
}

QString SettingsRepository::lastError() const
{
    return m_lastError;
}
