#pragma once

#include <QAbstractTableModel>
#include <QList>
#include "passwordentry.h"

class PasswordRepository;

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

    explicit PasswordTableModel(PasswordRepository *repository, QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    // Bulk reload from repository
    void reload();

    // Direct access for Delete / Copy
    PasswordEntry entryAt(int row) const;

    // Collect all categories currently in the list
    QStringList categories() const;

private:
    PasswordRepository   *m_repository;
    QList<PasswordEntry>  m_entries;
};
