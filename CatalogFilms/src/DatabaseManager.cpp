#include "DatabaseManager.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "catalog_films_conn");
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase("catalog_films_conn");
}

bool DatabaseManager::open(const QString &filePath)
{
    m_db.setDatabaseName(filePath);
    if (!m_db.open()) {
        qDebug() << "DatabaseManager: failed to open:" << m_db.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::initializeSchema()
{
    QSqlQuery query(m_db);

    bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS movies ("
        "  id      INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title   TEXT    NOT NULL DEFAULT '',"
        "  genre   TEXT             DEFAULT '',"
        "  year    INTEGER          DEFAULT 0,"
        "  status  TEXT    NOT NULL DEFAULT 'Хочу подивитися',"
        "  rating  INTEGER          DEFAULT 0,"
        "  comment TEXT             DEFAULT ''"
        ")"
    );
    if (!ok) {
        qDebug() << "DatabaseManager: movies schema error:" << query.lastError().text();
        return false;
    }

    ok = query.exec(
        "CREATE TABLE IF NOT EXISTS settings ("
        "  id              INTEGER PRIMARY KEY,"
        "  theme           TEXT    DEFAULT 'light',"
        "  sort_column     TEXT    DEFAULT 'title',"
        "  sort_order      TEXT    DEFAULT 'asc',"
        "  movies_per_page INTEGER DEFAULT 50"
        ")"
    );
    if (!ok) {
        qDebug() << "DatabaseManager: settings schema error:" << query.lastError().text();
        return false;
    }

    // Ensure a singleton settings row exists
    ok = query.exec(
        "INSERT OR IGNORE INTO settings (id, theme, sort_column, sort_order, movies_per_page) "
        "VALUES (1, 'light', 'title', 'asc', 50)"
    );
    if (!ok) {
        qDebug() << "DatabaseManager: settings seed error:" << query.lastError().text();
        return false;
    }
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

QString DatabaseManager::lastError() const
{
    return m_db.lastError().text();
}
