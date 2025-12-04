/**
 * @file CertificateManager.h
 * @brief Certificate lifecycle management: install, validate, expire checks, rotation.
 */

#pragma once

#include "domain/common/Result.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <vector>
#include <optional>

namespace zmon
{

    /**
     * @enum CertificateType
     * @brief Supported certificate types.
     */
    enum class CertificateType
    {
        TlsClient,
        TlsServer,
        CodeSigning
    };

    /**
     * @struct CertificateRecord
     * @brief In-memory representation of a certificate metadata.
     */
    struct CertificateRecord
    {
        int id{0};
        QString name;
        CertificateType type{CertificateType::TlsServer};
        QString pem; // PEM content
        QString issuer;
        QString subject;
        QDateTime notBefore;
        QDateTime notAfter;
        QDateTime installedAt;
        bool active{true};
    };

    class SQLiteCertificateRepository;

    /**
     * @class CertificateManager
     * @brief Manages certificate lifecycle including validation and rotation.
     */
    class CertificateManager : public QObject
    {
        Q_OBJECT
    public:
        explicit CertificateManager(SQLiteCertificateRepository *repo, QObject *parent = nullptr);

        /** Install or update a certificate record. */
        Result<int> install(const CertificateRecord &cert);

        /** Validate a certificate's basic properties and date range. */
        Result<void> validate(const CertificateRecord &cert) const;

        /** Check if a certificate expires within given days. Default 30. */
        bool isExpiringSoon(const CertificateRecord &cert, int days = 30) const;

        /** Rotate certificate if expiring soon: mark current inactive, install replacement. */
        Result<int> rotateIfNeeded(const QString &name, const CertificateRecord &replacement, int days = 30);

        /** Retrieve by name. */
        Result<std::optional<CertificateRecord>> getByName(const QString &name) const;

        /** List all certificates. */
        Result<std::vector<CertificateRecord>> listAll() const;

    private:
        SQLiteCertificateRepository *m_repo;
    };
}
