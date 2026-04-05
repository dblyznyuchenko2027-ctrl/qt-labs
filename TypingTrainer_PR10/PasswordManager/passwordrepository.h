#pragma once

#include <QList>
#include <QtSql/QSqlDatabase>
#include "passwordentry.h"

class PasswordRepository
{
public:
    explicit PasswordRepository(const QSqlDatabase &db);

    QList<PasswordEntry> loadAll() const;
    bool insert(PasswordEntry &entry);      // sets entry.id after insert
    bool update(const PasswordEntry &entry);
    bool remove(int id);

    QString lastError() const;

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};
