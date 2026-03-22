#include "csvticketrepository.h"

#include <QFile>
#include <QSaveFile>
#include <QTextStream>

CsvTicketRepository::CsvTicketRepository(const QString &filePath)
    : m_filePath(filePath)
{
}

QList<Ticket> CsvTicketRepository::loadAll() const
{
    QList<Ticket> items;

    QFile file(m_filePath);
    if (!file.exists())
        return items;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return items;

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    // Skip header line
    if (!in.atEnd())
        in.readLine();

    while (!in.atEnd())
    {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        items.append(parseRow(line));
    }

    return items;
}

bool CsvTicketRepository::saveAll(const QList<Ticket> &items) const
{
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Write header
    out << "Id,Title,Priority,Status,CreatedAt,Description\n";

    for (const Ticket &t : items)
        out << toCsvRow(t) << "\n";

    return file.commit();
}

Ticket CsvTicketRepository::parseRow(const QString &line) const
{
    // Simple CSV parser supporting quoted fields
    QStringList parts;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i)
    {
        QChar c = line[i];
        if (inQuotes)
        {
            if (c == '"')
            {
                if (i + 1 < line.size() && line[i + 1] == '"')
                {
                    field += '"';
                    ++i;
                }
                else
                {
                    inQuotes = false;
                }
            }
            else
            {
                field += c;
            }
        }
        else
        {
            if (c == '"')
            {
                inQuotes = true;
            }
            else if (c == ',')
            {
                parts.append(field);
                field.clear();
            }
            else
            {
                field += c;
            }
        }
    }
    parts.append(field);

    Ticket t;
    t.id          = parts.value(0).toInt();
    t.title       = parts.value(1);
    t.priority    = parts.value(2);
    t.status      = parts.value(3);
    t.createdAt   = QDateTime::fromString(parts.value(4), Qt::ISODate);
    t.description = parts.value(5);
    return t;
}

QString CsvTicketRepository::toCsvRow(const Ticket &ticket) const
{
    return QString("%1,%2,%3,%4,%5,%6")
        .arg(ticket.id)
        .arg(escapeCsvField(ticket.title))
        .arg(escapeCsvField(ticket.priority))
        .arg(escapeCsvField(ticket.status))
        .arg(ticket.createdAt.toString(Qt::ISODate))
        .arg(escapeCsvField(ticket.description));
}

QString CsvTicketRepository::escapeCsvField(const QString &field) const
{
    if (field.contains(',') || field.contains('"') || field.contains('\n'))
    {
        QString escaped = field;
        escaped.replace("\"", "\"\"");
        return "\"" + escaped + "\"";
    }
    return field;
}

QString CsvTicketRepository::unescapeCsvField(const QString &field) const
{
    if (field.startsWith('"') && field.endsWith('"'))
    {
        QString inner = field.mid(1, field.size() - 2);
        inner.replace("\"\"", "\"");
        return inner;
    }
    return field;
}
