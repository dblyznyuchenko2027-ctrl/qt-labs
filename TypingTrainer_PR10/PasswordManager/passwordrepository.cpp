#include "passwordrepository.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDateTime>
#include <QDebug>

PasswordRepository::PasswordRepository(const QSqlDatabase &db)
    : m_db(db)
{}

QList<PasswordEntry> PasswordRepository::loadAll() const
{
    QList<PasswordEntry> items;
    QSqlQuery query(m_db);

    if (!query.exec(
            "SELECT id, title, username, password, website, category, updated_at "
            "FROM passwords ORDER BY id")) {
        m_lastError = query.lastError().text();
        qDebug() << "PasswordRepository::loadAll error:" << m_lastError;
        return items;
    }

    while (query.next()) {
        PasswordEntry e;
        e.id        = query.value(0).toInt();
        e.title     = query.value(1).toString();
        e.username  = query.value(2).toString();
        e.password  = query.value(3).toString();
        e.website   = query.value(4).toString();
        e.category  = query.value(5).toString();
        e.updatedAt = QDateTime::fromString(query.value(6).toString(), Qt::ISODate);
        items.append(e);
    }
    return items;
}

bool PasswordRepository::insert(PasswordEntry &entry)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO passwords (title, username, password, website, category, updated_at) "
        "VALUES (:title, :username, :password, :website, :category, :updatedAt)"
    );
    query.bindValue(":title",     entry.title);
    query.bindValue(":username",  entry.username);
    query.bindValue(":password",  entry.password);
    query.bindValue(":website",   entry.website);
    query.bindValue(":category",  entry.category);
    query.bindValue(":updatedAt", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "PasswordRepository::insert error:" << m_lastError;
        return false;
    }
    entry.id = query.lastInsertId().toInt();
    entry.updatedAt = QDateTime::currentDateTime();
    return true;
}

bool PasswordRepository::update(const PasswordEntry &entry)
{
    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE passwords SET "
        "  title      = :title,"
        "  username   = :username,"
        "  password   = :password,"
        "  website    = :website,"
        "  category   = :category,"
        "  updated_at = :updatedAt "
        "WHERE id = :id"
    );
    query.bindValue(":title",     entry.title);
    query.bindValue(":username",  entry.username);
    query.bindValue(":password",  entry.password);
    query.bindValue(":website",   entry.website);
    query.bindValue(":category",  entry.category);
    query.bindValue(":updatedAt", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":id",        entry.id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "PasswordRepository::update error:" << m_lastError;
        return false;
    }
    return true;
}

bool PasswordRepository::remove(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        qDebug() << "PasswordRepository::remove error:" << m_lastError;
        return false;
    }
    return true;
}

QString PasswordRepository::lastError() const
{
    return m_lastError;
}
