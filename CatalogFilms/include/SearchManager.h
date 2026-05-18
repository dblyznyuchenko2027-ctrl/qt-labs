#pragma once

#include <QSortFilterProxyModel>

class SearchManager : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SearchManager(QObject *parent = nullptr);

    void setSearchText(const QString &text);
    void setStatusFilter(const QString &status);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchText;
    QString m_statusFilter;
};
