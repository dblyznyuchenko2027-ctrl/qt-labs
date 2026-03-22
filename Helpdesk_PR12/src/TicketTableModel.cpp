#include "TicketTableModel.h"

// ── Constructor ───────────────────────────────────────────────────────────────
TicketTableModel::TicketTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    seedDemoData();
}

// ── Demo data — 8 varied tickets ─────────────────────────────────────────────
void TicketTableModel::seedDemoData()
{
    struct Seed { const char *title; const char *desc; const char *status; const char *priority; int daysAgo; };
    static const Seed seeds[] = {
        { "Cannot connect to VPN",
          "The VPN client is not connecting as of this morning.\n"
          "I have tried restarting my computer and resetting\n"
          "the VPN application, but it still won't connect.",
          "Open", "High", 1 },
        { "Printer on 3rd floor offline",
          "The shared printer HP-3F is showing offline.\n"
          "Other users on the floor are affected as well.",
          "In Progress", "Medium", 3 },
        { "Outlook keeps crashing",
          "Microsoft Outlook crashes every time I open an attachment.\n"
          "Error code: 0x80070057.",
          "Open", "High", 2 },
        { "Request new monitor",
          "I need a second monitor for my workstation.",
          "Resolved", "Low", 10 },
        { "Wi-Fi drops in meeting room B",
          "Wireless connection is unstable during video calls.",
          "Open", "Critical", 0 },
        { "Slow performance on shared drive",
          "Accessing files on \\\\server\\share takes 30+ seconds.",
          "In Progress", "Medium", 5 },
        { "Password reset required",
          "User locked out after too many failed login attempts.",
          "Resolved", "Low", 7 },
        { "Install VS Code for dev team",
          "Please install VS Code on workstations DEV-01 through DEV-05.",
          "Closed", "Low", 14 },
    };

    for (const auto &s : seeds) {
        Ticket t     = Ticket::makeNew(m_nextId++);
        t.title       = QString::fromUtf8(s.title);
        t.description = QString::fromUtf8(s.desc);
        t.status      = QString::fromUtf8(s.status);
        t.priority    = QString::fromUtf8(s.priority);
        t.createdAt   = QDateTime::currentDateTime().addDays(-s.daysAgo);
        m_tickets.append(t);
    }
}

// ── Required overrides ────────────────────────────────────────────────────────

int TicketTableModel::rowCount(const QModelIndex &parent) const
{
    // For table models: return 0 when parent is valid (no tree nesting)
    return parent.isValid() ? 0 : m_tickets.size();
}

int TicketTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : COL_COUNT;
}

QVariant TicketTableModel::data(const QModelIndex &index, int role) const
{
    // 1. Always validate the index first
    if (!index.isValid()) return {};
    const int row = index.row();
    const int col = index.column();
    if (row < 0 || row >= m_tickets.size()) return {};
    if (col < 0 || col >= COL_COUNT)        return {};

    const Ticket &t = m_tickets.at(row);

    // 2. DisplayRole — the text shown in the cell
    if (role == Qt::DisplayRole) {
        switch (col) {
        case COL_ID:       return t.id;
        case COL_TITLE:    return t.title;
        case COL_PRIORITY: return t.priority;
        case COL_STATUS:   return t.status;
        case COL_CREATED:  return t.createdAt.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        }
    }

    // 3. ForegroundRole — colour-code status and priority for visual clarity
    if (role == Qt::ForegroundRole) {
        if (col == COL_STATUS) {
            if (t.status == QLatin1String("Open"))        return QColor(Qt::darkRed);
            if (t.status == QLatin1String("In Progress")) return QColor(0, 80, 180);
            if (t.status == QLatin1String("Resolved"))    return QColor(Qt::darkGreen);
            if (t.status == QLatin1String("Closed"))      return QColor(Qt::gray);
        }
        if (col == COL_PRIORITY) {
            if (t.priority == QLatin1String("Critical")) return QColor(Qt::red);
            if (t.priority == QLatin1String("High"))     return QColor(200, 100, 0);
            if (t.priority == QLatin1String("Low"))      return QColor(Qt::darkGray);
        }
    }

    // 4. TextAlignmentRole — centre the ID column
    if (role == Qt::TextAlignmentRole && col == COL_ID)
        return int(Qt::AlignHCenter | Qt::AlignVCenter);

    // 5. UserRole — return the full Ticket as QVariant (useful for dialogs)
    if (role == Qt::UserRole)
        return QVariant::fromValue(t);

    return {};
}

QVariant TicketTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Vertical) return section + 1;  // row numbers

    // Horizontal column headers
    switch (section) {
    case COL_ID:       return tr("ID");
    case COL_TITLE:    return tr("Title");
    case COL_PRIORITY: return tr("Priority");
    case COL_STATUS:   return tr("Status");
    case COL_CREATED:  return tr("Created At");
    }
    return {};
}

// ── CRUD methods ──────────────────────────────────────────────────────────────

void TicketTableModel::addTicket(const Ticket &t)
{
    // 1. Tell Qt a new row is about to appear at the end
    const int newRow = m_tickets.size();
    beginInsertRows(QModelIndex(), newRow, newRow);

    // 2. Mutate the data
    Ticket inserted = t;
    if (inserted.id <= 0)
        inserted.id = m_nextId;
    if (!inserted.createdAt.isValid())
        inserted.createdAt = QDateTime::currentDateTime();

    m_tickets.append(inserted);
    m_nextId = qMax(m_nextId, inserted.id + 1);

    // 3. Notify Qt the structural change is done
    endInsertRows();
}

void TicketTableModel::updateTicket(int row, const Ticket &t)
{
    if (row < 0 || row >= m_tickets.size()) return;

    // 1. Mutate data
    m_tickets[row] = t;

    // 2. Emit dataChanged for the entire updated row so QTableView repaints it
    const QModelIndex topLeft     = index(row, 0);
    const QModelIndex bottomRight = index(row, COL_COUNT - 1);
    emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole, Qt::ForegroundRole});
}

void TicketTableModel::removeTicket(int row)
{
    if (row < 0 || row >= m_tickets.size()) return;

    // 1. Announce removal
    beginRemoveRows(QModelIndex(), row, row);

    // 2. Remove
    m_tickets.removeAt(row);

    // 3. Done
    endRemoveRows();
}

// ── Accessor ──────────────────────────────────────────────────────────────────
Ticket TicketTableModel::ticketAt(int row) const
{
    if (row < 0 || row >= m_tickets.size()) return {};
    return m_tickets.at(row);
}
