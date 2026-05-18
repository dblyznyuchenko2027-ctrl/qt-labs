#include "SettingsManager.h"

SettingsManager::SettingsManager(SettingsRepository *repo)
    : m_repo(repo)
{
    m_current = m_repo->load();
}

Settings SettingsManager::currentSettings() const
{
    return m_current;
}

bool SettingsManager::saveSettings(const Settings &settings)
{
    if (!m_repo->save(settings)) {
        m_lastError = m_repo->lastError();
        return false;
    }
    m_current = settings;
    return true;
}

QString SettingsManager::lastError() const
{
    return m_lastError;
}
