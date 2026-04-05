#pragma once

#include <QAbstractTableModel>
#include <QList>
#include "passwordentry.h"

class PasswordTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColId = 0,
        ColTitle,
        ColUsername,
        ColPassword,
        ColWebsite,
        ColCategory,
        ColUpdatedAt,
        ColCount
    };

    explicit PasswordTableModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Data management
    void setEntries(const QList<PasswordEntry> &entries);
    PasswordEntry entryAt(int row) const;

    // Signal to notify MainWindow that a cell was edited in-place
signals:
    void entryEdited(const PasswordEntry &entry);

private:
    QList<PasswordEntry> m_entries;
};
