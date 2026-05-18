#include "SearchManager.h"
#include "MovieTableModel.h"

SearchManager::SearchManager(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void SearchManager::setSearchText(const QString &text)
{
    m_searchText = text.trimmed();
    invalidateFilter();
}

void SearchManager::setStatusFilter(const QString &status)
{
    m_statusFilter = status;
    invalidateFilter();
}

bool SearchManager::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QAbstractItemModel *src = sourceModel();
    if (!src) return true;

    if (!m_searchText.isEmpty()) {
        QString title = src->data(src->index(sourceRow, COL_TITLE, sourceParent)).toString();
        if (!title.contains(m_searchText, Qt::CaseInsensitive))
            return false;
    }

    if (!m_statusFilter.isEmpty() && m_statusFilter != "Усі") {
        QString status = src->data(src->index(sourceRow, COL_STATUS, sourceParent)).toString();
        if (status != m_statusFilter)
            return false;
    }
    return true;
}
