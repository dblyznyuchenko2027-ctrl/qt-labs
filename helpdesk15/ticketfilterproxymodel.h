#pragma once

#include <QSortFilterProxyModel>
#include <QString>

class TicketFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TicketFilterProxyModel(QObject *parent = nullptr);

    void setTextFilter(const QString &text);
    void setStatusFilter(const QString &status);
    void setPriorityFilter(const QString &priority);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_text;
    QString m_status;
    QString m_priority;
};
