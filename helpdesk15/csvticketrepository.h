#pragma once

#include <QList>
#include <QString>
#include "ticket.h"

class CsvTicketRepository
{
public:
    explicit CsvTicketRepository(const QString &filePath);

    QList<Ticket> loadAll() const;
    bool saveAll(const QList<Ticket> &items) const;

private:
    Ticket parseRow(const QString &line) const;
    QString toCsvRow(const Ticket &ticket) const;
    QString escapeCsvField(const QString &field) const;
    QString unescapeCsvField(const QString &field) const;

    QString m_filePath;
};
