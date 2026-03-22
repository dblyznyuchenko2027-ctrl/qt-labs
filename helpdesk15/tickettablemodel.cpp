#include "tickettablemodel.h"

TicketTableModel::TicketTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TicketTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

int TicketTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 5; // ID, Title, Priority, Status, Created At
}

QVariant TicketTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    if (role != Qt::DisplayRole)
        return {};

    const Ticket &t = m_items.at(index.row());
    switch (index.column())
    {
    case 0: return t.id;
    case 1: return t.title;
    case 2: return t.priority;
    case 3: return t.status;
    case 4: return t.createdAt.toString("yyyy-MM-dd hh:mm");
    default: return {};
    }
}

QVariant TicketTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return {};

    switch (section)
    {
    case 0: return "ID";
    case 1: return "Title";
    case 2: return "Priority";
    case 3: return "Status";
    case 4: return "Created At";
    default: return {};
    }
}

void TicketTableModel::setItems(const QList<Ticket> &items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void TicketTableModel::appendTicket(const Ticket &ticket)
{
    const int newRow = m_items.size();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_items.append(ticket);
    endInsertRows();
}

void TicketTableModel::replaceTicket(int row, const Ticket &ticket)
{
    if (row < 0 || row >= m_items.size())
        return;
    m_items[row] = ticket;
    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
}

void TicketTableModel::removeTicket(int row)
{
    if (row < 0 || row >= m_items.size())
        return;
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}

Ticket TicketTableModel::ticketAt(int row) const
{
    return m_items.at(row);
}

const QList<Ticket> &TicketTableModel::tickets() const
{
    return m_items;
}
