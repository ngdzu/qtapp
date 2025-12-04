/**
 * @file SQLiteCertificateRepository.h
 * @brief Persistence adapter for certificates.
 */

#pragma once

#include "infrastructure/security/CertificateManager.h"
#include "infrastructure/persistence/DatabaseManager.h"
#include "domain/common/Result.h"
#include <QObject>
#include <optional>

namespace zmon
{

    /**
     * @class SQLiteCertificateRepository
     * @brief CRUD operations for certificates in SQLite database.
     */
    class SQLiteCertificateRepository : public QObject
    {
        Q_OBJECT
    public:
        explicit SQLiteCertificateRepository(DatabaseManager *db, QObject *parent = nullptr);

        Result<int> upsert(const CertificateRecord &cert);
        Result<std::optional<CertificateRecord>> getByName(const QString &name) const;
        Result<std::vector<CertificateRecord>> listAll() const;

    private:
        DatabaseManager *m_db{nullptr};
        static CertificateRecord fromQuery(const QSqlQuery &q);
    };
}
