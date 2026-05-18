#include "MovieDialog.h"
#include "ui_MovieDialog.h"
#include "ValidationService.h"

#include <QMessageBox>

MovieDialog::MovieDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::MovieDialog)
{
    ui->setupUi(this);
    setupGenres();
    setupStatuses();

    connect(ui->saveButton,   &QPushButton::clicked, this, &MovieDialog::onSave);
    connect(ui->cancelButton, &QPushButton::clicked, this, &MovieDialog::onCancel);
}

MovieDialog::~MovieDialog()
{
    delete ui;
}

void MovieDialog::setupGenres()
{
    ui->genreCombo->addItems({
        "",
        QStringLiteral("Бойовик"),
        QStringLiteral("Комедія"),
        QStringLiteral("Драма"),
        QStringLiteral("Фантастика"),
        QStringLiteral("Жахи"),
        QStringLiteral("Мелодрама"),
        QStringLiteral("Трилер"),
        QStringLiteral("Документальний"),
        QStringLiteral("Анімація"),
        QStringLiteral("Інше"),
    });
}

void MovieDialog::setupStatuses()
{
    ui->statusCombo->addItems({
        QStringLiteral("Хочу подивитися"),
        QStringLiteral("Переглянуто"),
    });
}

void MovieDialog::setMovie(const Movie &movie)
{
    m_movieId = movie.id;
    setWindowTitle(movie.id > 0 ? tr("Редагування фільму") : tr("Додавання фільму"));

    ui->titleEdit->setText(movie.title);

    int genreIdx = ui->genreCombo->findText(movie.genre);
    if (genreIdx >= 0) {
        ui->genreCombo->setCurrentIndex(genreIdx);
    } else {
        ui->genreCombo->setEditText(movie.genre);
    }

    ui->yearSpin->setValue(movie.year > 0 ? movie.year : 2024);

    int statusIdx = ui->statusCombo->findText(movie.status);
    ui->statusCombo->setCurrentIndex(statusIdx >= 0 ? statusIdx : 0);

    ui->ratingSpin->setValue(movie.rating);
    ui->commentEdit->setPlainText(movie.comment);
}

Movie MovieDialog::movie() const
{
    Movie m;
    m.id      = m_movieId;
    m.title   = ui->titleEdit->text().trimmed();
    m.genre   = ui->genreCombo->currentText().trimmed();
    m.year    = ui->yearSpin->value();
    m.status  = ui->statusCombo->currentText();
    m.rating  = ui->ratingSpin->value();
    m.comment = ui->commentEdit->toPlainText().trimmed();
    return m;
}

void MovieDialog::onSave()
{
    Movie m = movie();
    QString err;
    if (!ValidationService::validateMovie(m, err)) {
        QMessageBox::warning(this, tr("Помилка валідації"), err);
        return;
    }
    accept();
}

void MovieDialog::onCancel()
{
    reject();
}
