#include "passwordtablemodel.h"
#include "passwordrepository.h"

#include <QDateTime>

PasswordTableModel::PasswordTableModel(PasswordRepository *repository, QObject *parent)
    : QAbstractTableModel(parent)
    , m_repository(repository)
{}

// ── Geometry ──────────────────────────────────────────────────────────────────

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

// ── Data ──────────────────────────────────────────────────────────────────────

QVariant PasswordTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const PasswordEntry &e = m_entries.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case ColId:       return e.id;
        case ColTitle:    return e.title;
        case ColUsername: return e.username;
        case ColPassword:
            return (role == Qt::DisplayRole) ? QString("••••••••") : e.password;
        case ColWebsite:  return e.website;
        case ColCategory: return e.category;
        case ColUpdatedAt:
            return e.updatedAt.isValid()
                ? e.updatedAt.toString("yyyy-MM-dd HH:mm")
                : QString();
        }
    }

    // Tooltip for password column
    if (role == Qt::ToolTipRole && index.column() == ColPassword)
        return "Double-click to edit password";

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

// ── Flags & editing ───────────────────────────────────────────────────────────

Qt::ItemFlags PasswordTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    // ID and UpdatedAt are read-only
    if (index.column() == ColId || index.column() == ColUpdatedAt)
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool PasswordTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    PasswordEntry updated = m_entries.at(index.row());

    switch (index.column()) {
    case ColTitle:    updated.title    = value.toString(); break;
    case ColUsername: updated.username = value.toString(); break;
    case ColPassword: updated.password = value.toString(); break;
    case ColWebsite:  updated.website  = value.toString(); break;
    case ColCategory: updated.category = value.toString(); break;
    default: return false;
    }
    updated.updatedAt = QDateTime::currentDateTime();

    // Persist to DB through repository
    if (!m_repository->update(updated))
        return false;

    m_entries[index.row()] = updated;
    emit dataChanged(index, index.siblingAtColumn(ColUpdatedAt),
                     {Qt::DisplayRole, Qt::EditRole});
    return true;
}

// ── Bulk operations ───────────────────────────────────────────────────────────

void PasswordTableModel::reload()
{
    beginResetModel();
    m_entries = m_repository->loadAll();
    endResetModel();
}

PasswordEntry PasswordTableModel::entryAt(int row) const
{
    if (row < 0 || row >= m_entries.size()) return {};
    return m_entries.at(row);
}

QStringList PasswordTableModel::categories() const
{
    QStringList cats;
    for (const PasswordEntry &e : m_entries) {
        if (!e.category.isEmpty() && !cats.contains(e.category))
            cats.append(e.category);
    }
    cats.sort(Qt::CaseInsensitive);
    return cats;
}
