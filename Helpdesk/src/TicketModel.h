#pragma once
#include <QAbstractTableModel>
#include <QDateTime>
#include <QList>
#include <QString>

// ── Ticket data structure ─────────────────────────────────────────────────────
struct Ticket {
    int       id          = 0;
    QString   title;
    QString   description;
    QString   status;      // Open / In Progress / Resolved / Closed
    QString   priority;    // Low / Medium / High / Critical
    QDateTime createdAt;

    static Ticket makeNew(int id) {
        Ticket t;
        t.id        = id;
        t.status    = QStringLiteral("Open");
        t.priority  = QStringLiteral("Medium");
        t.createdAt = QDateTime::currentDateTime();
        return t;
    }
};

// ── Table columns ─────────────────────────────────────────────────────────────
enum TicketColumn {
    COL_ID = 0,
    COL_TITLE,
    COL_PRIORITY,
    COL_STATUS,
    COL_CREATED,
    COL_COUNT
};

// ── QAbstractTableModel ───────────────────────────────────────────────────────
class TicketModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit TicketModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int      rowCount(const QModelIndex &parent = {}) const override;
    int      columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // CRUD helpers
    void      addTicket(const Ticket &t);
    void      updateTicket(int row, const Ticket &t);
    void      removeTicket(int row);
    Ticket    ticketAt(int row) const;
    int       nextId() const { return m_nextId; }

private:
    QList<Ticket> m_tickets;
    int           m_nextId = 1;

    void seedData();  // add demo rows
};
