#pragma once

#include <QDialog>
#include "Movie.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MovieDialog; }
QT_END_NAMESPACE

class MovieDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MovieDialog(QWidget *parent = nullptr);
    ~MovieDialog() override;

    void setMovie(const Movie &movie);
    Movie movie() const;

private slots:
    void onSave();
    void onCancel();

private:
    void setupGenres();
    void setupStatuses();

    Ui::MovieDialog *ui;
    int m_movieId = 0;
};
