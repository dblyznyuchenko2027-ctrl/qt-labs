#pragma once

#include <QTableView>

class MovieTableWidget : public QTableView
{
    Q_OBJECT
public:
    explicit MovieTableWidget(QWidget *parent = nullptr);

    void setupColumns();
};
