#include "passwordtablemodel.h"

PasswordTableModel::PasswordTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

int PasswordTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_entries.size();
}

int PasswordTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

QVariant PasswordTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const PasswordEntry &e = m_entries[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColId:        return e.id;
        case ColTitle:     return e.title;
        case ColUsername:  return e.username;
        case ColPassword:
            if (role == Qt::DisplayRole) return QString("••••••••");
            return e.password;
        case ColWebsite:   return e.website;
        case ColCategory:  return e.category;
        case ColUpdatedAt: return e.updatedAt.toString("yyyy-MM-dd HH:mm");
        }
    }
    return {};
}

QVariant PasswordTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return {};
    if (orientation == Qt::Vertical) return section + 1;

    switch (section) {
    case ColId:        return "ID";
    case ColTitle:     return "Title";
    case ColUsername:  return "Username";
    case ColPassword:  return "Password";
    case ColWebsite:   return "Website";
    case ColCategory:  return "Category";
    case ColUpdatedAt: return "Updated At";
    }
    return {};
}

Qt::ItemFlags PasswordTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    if (index.column() == ColId) return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool PasswordTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    PasswordEntry &e = m_entries[index.row()];
    switch (index.column()) {
    case ColTitle:    e.title    = value.toString(); break;
    case ColUsername: e.username = value.toString(); break;
    case ColPassword: e.password = value.toString(); break;
    case ColWebsite:  e.website  = value.toString(); break;
    case ColCategory: e.category = value.toString(); break;
    default: return false;
    }
    e.updatedAt = QDateTime::currentDateTime();
    emit dataChanged(index, index.siblingAtColumn(ColUpdatedAt));
    return true;
}

void PasswordTableModel::addEntry(const PasswordEntry &entry)
{
    PasswordEntry e = entry;
    e.id = m_nextId++;
    if (!e.updatedAt.isValid())
        e.updatedAt = QDateTime::currentDateTime();

    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.append(e);
    endInsertRows();
}

void PasswordTableModel::removeEntry(int row)
{
    if (row < 0 || row >= m_entries.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_entries.removeAt(row);
    endRemoveRows();
}

PasswordEntry PasswordTableModel::entryAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) return {};
    return m_entries[row];
}
