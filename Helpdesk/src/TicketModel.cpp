#include "TicketModel.h"

TicketModel::TicketModel(QObject *parent) : QAbstractTableModel(parent)
{
    seedData();
}

void TicketModel::seedData()
{
    auto add = [&](const QString &title, const QString &desc,
                   const QString &status, const QString &priority) {
        Ticket t = Ticket::makeNew(m_nextId++);
        t.title       = title;
        t.description = desc;
        t.status      = status;
        t.priority    = priority;
        t.createdAt   = QDateTime::currentDateTime().addDays(-(m_nextId * 2));
        m_tickets.append(t);
    };

    add("Cannot connect to VPN",
        "The VPN client is not connecting as of this morning. I have tried\n"
        "restarting my computer and resetting the VPN application, but\n"
        "it still won't connect.",
        "Open", "High");

    add("Printer on 3rd floor offline",
        "The shared printer HP-3F is showing offline status.\n"
        "Other users on the floor are affected as well.",
        "In Progress", "Medium");

    add("Outlook keeps crashing",
        "Microsoft Outlook crashes every time I try to open an attachment.\n"
        "Error code: 0x80070057.",
        "Open", "High");

    add("Request new monitor",
        "I need a second monitor for my workstation to improve productivity.",
        "Resolved", "Low");

    add("Wi-Fi drops in meeting room B",
        "The wireless connection is unstable in meeting room B during video calls.",
        "Open", "Critical");
}

int TicketModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_tickets.size();
}

int TicketModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return COL_COUNT;
}

QVariant TicketModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tickets.size()) return {};

    const Ticket &t = m_tickets.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case COL_ID:      return t.id;
        case COL_TITLE:   return t.title;
        case COL_PRIORITY:return t.priority;
        case COL_STATUS:  return t.status;
        case COL_CREATED: return t.createdAt.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        }
    }

    // Colour-code status column
    if (role == Qt::ForegroundRole && index.column() == COL_STATUS) {
        if (t.status == "Open")        return QColor(Qt::darkRed);
        if (t.status == "In Progress") return QColor(Qt::darkBlue);
        if (t.status == "Resolved")    return QColor(Qt::darkGreen);
        if (t.status == "Closed")      return QColor(Qt::gray);
    }

    // Colour-code priority column
    if (role == Qt::ForegroundRole && index.column() == COL_PRIORITY) {
        if (t.priority == "Critical") return QColor(Qt::red);
        if (t.priority == "High")     return QColor(200, 100, 0);
    }

    if (role == Qt::TextAlignmentRole && index.column() == COL_ID)
        return Qt::AlignCenter;

    return {};
}

QVariant TicketModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return {};
    switch (section) {
    case COL_ID:       return tr("ID");
    case COL_TITLE:    return tr("Title");
    case COL_PRIORITY: return tr("Priority");
    case COL_STATUS:   return tr("Status");
    case COL_CREATED:  return tr("Created At");
    }
    return {};
}

void TicketModel::addTicket(const Ticket &t)
{
    beginInsertRows({}, m_tickets.size(), m_tickets.size());
    m_tickets.append(t);
    ++m_nextId;
    endInsertRows();
}

void TicketModel::updateTicket(int row, const Ticket &t)
{
    if (row < 0 || row >= m_tickets.size()) return;
    m_tickets[row] = t;
    emit dataChanged(index(row, 0), index(row, COL_COUNT - 1));
}

void TicketModel::removeTicket(int row)
{
    if (row < 0 || row >= m_tickets.size()) return;
    beginRemoveRows({}, row, row);
    m_tickets.removeAt(row);
    endRemoveRows();
}

Ticket TicketModel::ticketAt(int row) const
{
    if (row < 0 || row >= m_tickets.size()) return {};
    return m_tickets.at(row);
}
