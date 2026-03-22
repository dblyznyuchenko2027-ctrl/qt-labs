#pragma once
#include <QAbstractTableModel>
#include <QDateTime>
#include <QString>
#include <QVector>

// ─────────────────────────────────────────────────────────────────────────────
//  Ticket — plain data structure (no Qt base class, just data)
// ─────────────────────────────────────────────────────────────────────────────
struct Ticket {
    int       id          = 0;
    QString   title;
    QString   description;
    QString   priority;   // Low | Medium | High | Critical
    QString   status;     // Open | In Progress | Resolved | Closed
    QDateTime createdAt;

    bool isValid() const { return id > 0; }

    static Ticket makeNew(int id) {
        Ticket t;
        t.id        = id;
        t.priority  = QStringLiteral("Medium");
        t.status    = QStringLiteral("Open");
        t.createdAt = QDateTime::currentDateTime();
        return t;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Column enum — single source of truth for column order
// ─────────────────────────────────────────────────────────────────────────────
enum TicketColumn {
    COL_ID = 0,
    COL_TITLE,
    COL_PRIORITY,
    COL_STATUS,
    COL_CREATED,
    COL_COUNT       // sentinel — must stay last
};

// ─────────────────────────────────────────────────────────────────────────────
//  TicketTableModel — custom QAbstractTableModel for PR12
//
//  Responsibilities (Model side only):
//    - store QVector<Ticket>
//    - implement rowCount / columnCount / data / headerData
//    - emit correct begin/end signals on structural changes
//    - emit dataChanged on value changes
//
//  NOT responsible for:
//    - UI (no QWidget knowledge)
//    - dialogs, selections, actions
// ─────────────────────────────────────────────────────────────────────────────
class TicketTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TicketTableModel(QObject *parent = nullptr);

    // ── QAbstractTableModel required overrides ────────────────────────────
    int      rowCount   (const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data       (const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const override;

    // ── CRUD — always use these; never modify m_tickets directly ─────────
    void   addTicket   (const Ticket &t);
    void   updateTicket(int row, const Ticket &t);
    void   removeTicket(int row);

    // ── Accessors ─────────────────────────────────────────────────────────
    Ticket ticketAt(int row) const;
    int    nextId()  const { return m_nextId; }
    int    count()   const { return m_tickets.size(); }

private:
    QVector<Ticket> m_tickets;
    int             m_nextId = 1;

    void seedDemoData();
};
