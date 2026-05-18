#pragma once

#include <QDialog>
#include "Settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWindow; }
QT_END_NAMESPACE

class SettingsWindow : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow() override;

    void setSettings(const Settings &settings);
    Settings settings() const;

private slots:
    void onSave();

private:
    Ui::SettingsWindow *ui;
};
