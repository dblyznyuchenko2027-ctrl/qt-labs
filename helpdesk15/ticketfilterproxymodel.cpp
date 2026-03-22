#include "ticketfilterproxymodel.h"

TicketFilterProxyModel::TicketFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

void TicketFilterProxyModel::setTextFilter(const QString &text)
{
    m_text = text;
    invalidateFilter();
}

void TicketFilterProxyModel::setStatusFilter(const QString &status)
{
    m_status = status;
    invalidateFilter();
}

void TicketFilterProxyModel::setPriorityFilter(const QString &priority)
{
    m_priority = priority;
    invalidateFilter();
}

bool TicketFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // Column indices: 0=ID, 1=Title, 2=Priority, 3=Status, 4=CreatedAt
    const QModelIndex titleIndex    = sourceModel()->index(sourceRow, 1, sourceParent);
    const QModelIndex priorityIndex = sourceModel()->index(sourceRow, 2, sourceParent);
    const QModelIndex statusIndex   = sourceModel()->index(sourceRow, 3, sourceParent);

    const QString title    = sourceModel()->data(titleIndex).toString();
    const QString priority = sourceModel()->data(priorityIndex).toString();
    const QString status   = sourceModel()->data(statusIndex).toString();

    const bool textOk     = m_text.isEmpty()     || title.contains(m_text, Qt::CaseInsensitive);
    const bool statusOk   = m_status.isEmpty()   || status == m_status;
    const bool priorityOk = m_priority.isEmpty() || priority == m_priority;

    return textOk && statusOk && priorityOk;
}
