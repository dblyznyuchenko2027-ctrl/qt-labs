#pragma once

#include <QString>
#include <QtSql/QSqlDatabase>

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString &filePath);
    bool initializeSchema();
    QSqlDatabase database() const;
    QString lastError() const;

private:
    QSqlDatabase m_db;
};
