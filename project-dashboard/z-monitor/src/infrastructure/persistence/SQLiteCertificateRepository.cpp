#include "infrastructure/persistence/SQLiteCertificateRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace zmon
{

    SQLiteCertificateRepository::SQLiteCertificateRepository(DatabaseManager *db, QObject *parent)
        : QObject(parent), m_db(db) {}

    Result<int> SQLiteCertificateRepository::upsert(const CertificateRecord &cert)
    {
        qDebug() << "[SQLiteCertificateRepository::upsert] Starting upsert for:" << cert.name;
        QSqlDatabase db = m_db->getWriteConnection();
        qDebug() << "[SQLiteCertificateRepository::upsert] Got write connection, valid:" << db.isValid();

        // First, check if it exists
        QSqlQuery checkQuery(db);
        checkQuery.prepare("SELECT id FROM certificate_store WHERE name=?");
        checkQuery.addBindValue(cert.name);
        bool exists = false;
        int existingId = -1;
        if (checkQuery.exec() && checkQuery.next())
        {
            exists = true;
            existingId = checkQuery.value(0).toInt();
        }

        if (exists)
        {
            // UPDATE
            qDebug() << "[SQLiteCertificateRepository::upsert] Updating existing record, id:" << existingId;
            QSqlQuery updateQuery(db);
            updateQuery.prepare("UPDATE certificate_store SET type=?, pem=?, issuer=?, subject=?, not_before=?, not_after=?, installed_at=?, active=? WHERE name=?");
            updateQuery.addBindValue(static_cast<int>(cert.type));
            updateQuery.addBindValue(cert.pem);
            updateQuery.addBindValue(cert.issuer);
            updateQuery.addBindValue(cert.subject);
            updateQuery.addBindValue(cert.notBefore.toString(Qt::ISODateWithMs));
            updateQuery.addBindValue(cert.notAfter.toString(Qt::ISODateWithMs));
            updateQuery.addBindValue(cert.installedAt.toString(Qt::ISODateWithMs));
            updateQuery.addBindValue(cert.active);
            updateQuery.addBindValue(cert.name);
            if (!updateQuery.exec())
            {
                qDebug() << "[SQLiteCertificateRepository::upsert] Update failed:" << updateQuery.lastError().text();
                return Result<int>::error(Error::create(ErrorCode::DatabaseError, updateQuery.lastError().text().toStdString()));
            }
            qDebug() << "[SQLiteCertificateRepository::upsert] Update succeeded, id:" << existingId;
            return Result<int>::ok(existingId);
        }
        else
        {
            // INSERT
            qDebug() << "[SQLiteCertificateRepository::upsert] Inserting new record";
            QSqlQuery insertQuery(db);
            insertQuery.prepare("INSERT INTO certificate_store(name,type,pem,issuer,subject,not_before,not_after,installed_at,active)"
                                " VALUES(?,?,?,?,?,?,?,?,?)");
            insertQuery.addBindValue(cert.name);
            insertQuery.addBindValue(static_cast<int>(cert.type));
            insertQuery.addBindValue(cert.pem);
            insertQuery.addBindValue(cert.issuer);
            insertQuery.addBindValue(cert.subject);
            insertQuery.addBindValue(cert.notBefore.toString(Qt::ISODateWithMs));
            insertQuery.addBindValue(cert.notAfter.toString(Qt::ISODateWithMs));
            insertQuery.addBindValue(cert.installedAt.toString(Qt::ISODateWithMs));
            insertQuery.addBindValue(cert.active);
            if (!insertQuery.exec())
            {
                qDebug() << "[SQLiteCertificateRepository::upsert] Insert failed:" << insertQuery.lastError().text();
                return Result<int>::error(Error::create(ErrorCode::DatabaseError, insertQuery.lastError().text().toStdString()));
            }
            int newId = insertQuery.lastInsertId().toInt();
            qDebug() << "[SQLiteCertificateRepository::upsert] Insert succeeded, id:" << newId;
            return Result<int>::ok(newId);
        }
    }

    Result<std::optional<CertificateRecord>> SQLiteCertificateRepository::getByName(const QString &name) const
    {
        qDebug() << "[SQLiteCertificateRepository::getByName] Fetching certificate:" << name;
        QSqlDatabase db = m_db->getReadConnection();
        qDebug() << "[SQLiteCertificateRepository::getByName] Got read connection, valid:" << db.isValid();
        QSqlQuery q(db);
        q.prepare("SELECT id,name,type,pem,issuer,subject,not_before,not_after,installed_at,active FROM certificate_store WHERE name=:name LIMIT 1");
        q.bindValue(":name", name);
        if (!q.exec())
        {
            qDebug() << "[SQLiteCertificateRepository::getByName] Query exec failed:" << q.lastError().text();
            return Result<std::optional<CertificateRecord>>::error(Error::create(ErrorCode::DatabaseError, q.lastError().text().toStdString()));
        }
        if (q.next())
        {
            qDebug() << "[SQLiteCertificateRepository::getByName] Certificate found";
            return Result<std::optional<CertificateRecord>>::ok(std::optional<CertificateRecord>{fromQuery(q)});
        }
        qDebug() << "[SQLiteCertificateRepository::getByName] Certificate not found";
        return Result<std::optional<CertificateRecord>>::ok(std::optional<CertificateRecord>{});
    }

    Result<std::vector<CertificateRecord>> SQLiteCertificateRepository::listAll() const
    {
        QSqlDatabase db = m_db->getReadConnection();
        QSqlQuery q(db);
        if (!q.exec("SELECT id,name,type,pem,issuer,subject,not_before,not_after,installed_at,active FROM certificate_store ORDER BY name"))
        {
            return Result<std::vector<CertificateRecord>>::error(Error::create(ErrorCode::DatabaseError, q.lastError().text().toStdString()));
        }
        std::vector<CertificateRecord> out;
        while (q.next())
        {
            out.push_back(fromQuery(q));
        }
        return Result<std::vector<CertificateRecord>>::ok(std::move(out));
    }

    CertificateRecord SQLiteCertificateRepository::fromQuery(const QSqlQuery &q)
    {
        CertificateRecord c;
        c.id = q.value(0).toInt();
        c.name = q.value(1).toString();
        c.type = static_cast<CertificateType>(q.value(2).toInt());
        c.pem = q.value(3).toString();
        c.issuer = q.value(4).toString();
        c.subject = q.value(5).toString();
        c.notBefore = QDateTime::fromString(q.value(6).toString(), Qt::ISODateWithMs);
        c.notAfter = QDateTime::fromString(q.value(7).toString(), Qt::ISODateWithMs);
        c.installedAt = QDateTime::fromString(q.value(8).toString(), Qt::ISODateWithMs);
        c.active = q.value(9).toBool();
        return c;
    }

}
