#include "infrastructure/security/CertificateManager.h"
#include "infrastructure/persistence/SQLiteCertificateRepository.h"
#include <QSslCertificate>
#include <QDebug>

namespace zmon
{

    CertificateManager::CertificateManager(SQLiteCertificateRepository *repo, QObject *parent)
        : QObject(parent), m_repo(repo) {}

    Result<int> CertificateManager::install(const CertificateRecord &cert)
    {
        qDebug() << "[CertificateManager::install] Starting install for:" << cert.name;
        if (auto v = validate(cert); v.isError())
        {
            qDebug() << "[CertificateManager::install] Validation failed:" << QString::fromStdString(v.error().message);
            return Result<int>::error(v.error());
        }
        qDebug() << "[CertificateManager::install] Validation passed, calling upsert";
        auto result = m_repo->upsert(cert);
        if (result.isError())
        {
            qDebug() << "[CertificateManager::install] Upsert failed:" << QString::fromStdString(result.error().message);
        }
        else
        {
            qDebug() << "[CertificateManager::install] Upsert succeeded, id:" << result.value();
        }
        return result;
    }

    Result<void> CertificateManager::validate(const CertificateRecord &cert) const
    {
        qDebug() << "[CertificateManager::validate] Validating certificate:" << cert.name;
        if (cert.pem.isEmpty())
        {
            qDebug() << "[CertificateManager::validate] Error: Empty PEM";
            return Result<void>::error(Error::create(ErrorCode::InvalidArgument, "Empty PEM"));
        }
        const QList<QSslCertificate> chain = QSslCertificate::fromData(cert.pem.toUtf8(), QSsl::Pem);
        qDebug() << "[CertificateManager::validate] Parsed PEM chain, size:" << chain.size();
        if (chain.isEmpty())
        {
            qDebug() << "[CertificateManager::validate] Error: Invalid PEM format";
            return Result<void>::error(Error::create(ErrorCode::InvalidArgument, "Invalid PEM format"));
        }
        if (!cert.notBefore.isValid() || !cert.notAfter.isValid())
        {
            qDebug() << "[CertificateManager::validate] Error: Invalid validity period";
            return Result<void>::error(Error::create(ErrorCode::InvalidArgument, "Invalid validity period"));
        }
        if (cert.notAfter <= cert.notBefore)
        {
            qDebug() << "[CertificateManager::validate] Error: notAfter must be after notBefore";
            return Result<void>::error(Error::create(ErrorCode::InvalidArgument, "notAfter must be after notBefore"));
        }
        qDebug() << "[CertificateManager::validate] Validation successful";
        return Result<void>::ok();
    }

    bool CertificateManager::isExpiringSoon(const CertificateRecord &cert, int days) const
    {
        const auto now = QDateTime::currentDateTimeUtc();
        const auto threshold = now.addDays(days);
        return cert.notAfter <= threshold;
    }

    Result<int> CertificateManager::rotateIfNeeded(const QString &name, const CertificateRecord &replacement, int days)
    {
        auto currentRes = m_repo->getByName(name);
        if (currentRes.isError())
            return Result<int>::error(currentRes.error());
        auto currentOpt = currentRes.value();
        if (currentOpt.has_value())
        {
            auto current = currentOpt.value();
            if (!isExpiringSoon(current, days))
            {
                return Result<int>::error(Error::create(ErrorCode::Conflict, "Current certificate not expiring soon"));
            }
            current.active = false;
            (void)m_repo->upsert(current);
        }
        return install(replacement);
    }

    Result<std::optional<CertificateRecord>> CertificateManager::getByName(const QString &name) const
    {
        return m_repo->getByName(name);
    }

    Result<std::vector<CertificateRecord>> CertificateManager::listAll() const
    {
        return m_repo->listAll();
    }

}
