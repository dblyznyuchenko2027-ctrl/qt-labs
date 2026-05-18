#include "SettingsWindow.h"
#include "ui_SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
    connect(ui->saveButton, &QPushButton::clicked, this, &SettingsWindow::onSave);
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::setSettings(const Settings &settings)
{
    ui->themeCombo->setCurrentIndex(settings.theme == "dark" ? 1 : 0);

    int sortIdx = 0;
    if (settings.sortColumn == "year")        sortIdx = 1;
    else if (settings.sortColumn == "rating") sortIdx = 2;
    else if (settings.sortColumn == "genre")  sortIdx = 3;
    ui->sortColumnCombo->setCurrentIndex(sortIdx);

    ui->sortOrderCombo->setCurrentIndex(settings.sortOrder == "desc" ? 1 : 0);
    ui->perPageSpin->setValue(settings.moviesPerPage);
}

Settings SettingsWindow::settings() const
{
    Settings s;
    s.theme = ui->themeCombo->currentIndex() == 1 ? "dark" : "light";

    switch (ui->sortColumnCombo->currentIndex()) {
    case 1: s.sortColumn = "year";   break;
    case 2: s.sortColumn = "rating"; break;
    case 3: s.sortColumn = "genre";  break;
    default: s.sortColumn = "title";
    }

    s.sortOrder     = ui->sortOrderCombo->currentIndex() == 1 ? "desc" : "asc";
    s.moviesPerPage = ui->perPageSpin->value();
    return s;
}

void SettingsWindow::onSave()
{
    accept();
}
