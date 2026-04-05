#include "passwordfilterproxymodel.h"
#include "passwordtablemodel.h"

PasswordFilterProxyModel::PasswordFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

void PasswordFilterProxyModel::setTextFilter(const QString &text)
{
    m_text = text;
    invalidateFilter();
}

void PasswordFilterProxyModel::setCategoryFilter(const QString &category)
{
    m_category = category;
    invalidateFilter();
}

bool PasswordFilterProxyModel::filterAcceptsRow(int sourceRow,
                                                const QModelIndex &sourceParent) const
{
    QAbstractItemModel *src = sourceModel();

    auto cell = [&](int col) {
        return src->data(src->index(sourceRow, col, sourceParent)).toString();
    };

    // Text filter: check Title, Username, Website
    if (!m_text.isEmpty()) {
        const bool titleMatch    = cell(PasswordTableModel::ColTitle).contains(m_text, Qt::CaseInsensitive);
        const bool usernameMatch = cell(PasswordTableModel::ColUsername).contains(m_text, Qt::CaseInsensitive);
        const bool websiteMatch  = cell(PasswordTableModel::ColWebsite).contains(m_text, Qt::CaseInsensitive);
        if (!titleMatch && !usernameMatch && !websiteMatch)
            return false;
    }

    // Category filter
    if (!m_category.isEmpty()) {
        if (cell(PasswordTableModel::ColCategory) != m_category)
            return false;
    }

    return true;
}
