#pragma once

#include <QSortFilterProxyModel>
#include <QString>

class PasswordFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit PasswordFilterProxyModel(QObject *parent = nullptr);

    void setTextFilter(const QString &text);
    void setCategoryFilter(const QString &category); // empty = show all

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_text;
    QString m_category;
};
