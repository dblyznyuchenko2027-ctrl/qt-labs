#pragma once

#include "Settings.h"
#include "SettingsRepository.h"

class SettingsManager
{
public:
    explicit SettingsManager(SettingsRepository *repo);

    Settings currentSettings() const;
    bool saveSettings(const Settings &settings);
    QString lastError() const;

private:
    SettingsRepository *m_repo;
    Settings m_current;
    QString m_lastError;
};
