#pragma once

#include <QtSql/QSqlDatabase>
#include "Settings.h"

class SettingsRepository
{
public:
    explicit SettingsRepository(const QSqlDatabase &db);

    Settings load() const;
    bool save(const Settings &settings);
    QString lastError() const;

private:
    QSqlDatabase m_db;
    mutable QString m_lastError;
};
