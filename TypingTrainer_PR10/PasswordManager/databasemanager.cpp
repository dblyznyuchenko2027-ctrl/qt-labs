#include "databasemanager.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

DatabaseManager::DatabaseManager()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "password_manager_conn");
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
    QSqlDatabase::removeDatabase("password_manager_conn");
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
        "CREATE TABLE IF NOT EXISTS passwords ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  title       TEXT    NOT NULL DEFAULT '',"
        "  username    TEXT             DEFAULT '',"
        "  password    TEXT             DEFAULT '',"
        "  website     TEXT             DEFAULT '',"
        "  category    TEXT             DEFAULT '',"
        "  updated_at  TEXT             DEFAULT ''"
        ")"
    );
    if (!ok)
        qDebug() << "DatabaseManager: initializeSchema error:" << query.lastError().text();
    return ok;
}

QSqlDatabase DatabaseManager::database() const
{
    return m_db;
}

QString DatabaseManager::lastError() const
{
    return m_db.lastError().text();
}
