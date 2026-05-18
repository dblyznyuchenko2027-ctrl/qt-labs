#pragma once

#include <QAbstractTableModel>
#include <QList>
#include "Movie.h"

enum MovieColumn {
    COL_ID = 0,
    COL_TITLE,
    COL_GENRE,
    COL_YEAR,
    COL_STATUS,
    COL_RATING,
    COL_COUNT
};

class MovieTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MovieTableModel(QObject *parent = nullptr);

    int      rowCount(const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setMovies(const QList<Movie> &movies);
    void addMovie(const Movie &movie);
    void updateMovie(int row, const Movie &movie);
    void removeMovie(int row);

    Movie movieAt(int row) const;
    int   count() const { return m_movies.size(); }

private:
    QList<Movie> m_movies;
};
