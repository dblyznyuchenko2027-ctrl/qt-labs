#include "MovieTableWidget.h"
#include "MovieTableModel.h"

#include <QHeaderView>

MovieTableWidget::MovieTableWidget(QWidget *parent)
    : QTableView(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setAlternatingRowColors(true);
    setSortingEnabled(true);
}

void MovieTableWidget::setupColumns()
{
    horizontalHeader()->setSectionResizeMode(COL_TITLE, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(COL_GENRE, QHeaderView::ResizeToContents);
    horizontalHeader()->setSectionResizeMode(COL_STATUS, QHeaderView::ResizeToContents);
    verticalHeader()->setVisible(false);
}
