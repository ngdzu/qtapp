#include <gtest/gtest.h>
#include "infrastructure/security/CertificateManager.h"
#include "infrastructure/persistence/SQLiteCertificateRepository.h"
#include "tests/fixtures/DatabaseTestFixture.h"

using namespace zmon;

class CertificateManagerFixture : public zmon::test::DatabaseTestFixture
{
protected:
    void SetUp() override
    {
        zmon::test::DatabaseTestFixture::SetUp();
        repo = new SQLiteCertificateRepository(databaseManager());
        mgr = std::make_unique<CertificateManager>(repo);
    }
    void TearDown() override
    {
        delete repo;
        mgr.reset();
        zmon::test::DatabaseTestFixture::TearDown();
    }
    SQLiteCertificateRepository *repo{nullptr};
    std::unique_ptr<CertificateManager> mgr;
};

TEST_F(CertificateManagerFixture, InstallAndFetch)
{
    CertificateRecord cert;
    cert.name = "tls-server";
    cert.type = CertificateType::TlsServer;
    // Valid self-signed test certificate (generated with openssl)
    cert.pem = R"(-----BEGIN CERTIFICATE-----
MIIDDzCCAfegAwIBAgIULdr6xt77NydwjMZGbRy3XuqRZaMwDQYJKoZIhvcNAQEL
BQAwFzEVMBMGA1UEAwwMWk1vbml0b3JUZXN0MB4XDTI1MTIwNDE0MDMzNVoXDTI2
MTIwNDE0MDMzNVowFzEVMBMGA1UEAwwMWk1vbml0b3JUZXN0MIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnPxKT7499qUlcDVX786DRlO/Bo+241bEC+2x
KQbBHvrlmYAQ2tAwiK0ec8Wy9YgKTzIqQcwevr2Z2LjaEMb9aHfECd+EKLv71b2Q
9ENkdx5GDVYH5o0rARV0edaOsIxMKETv3PuI7grpyhdAE942Z2wZ+yCNL8whSnEX
2KWbOjuyGQa/3+pKoJZTTXSB7q9gOL0gOoqDvNWMxNkim0J8oupXHaigD1bTpnwp
NWunGu0ti5UrR74ZK5fbtoKH5YSOx60suug9eQStnZ1FC8oAivYmDnHogmlBkc7H
+oLHq7VtAiHLxl5FfSvdVXds/F1Sr7KoJblyPgEUMYJR3BCBAQIDAQABo1MwUTAd
BgNVHQ4EFgQUfc9m2WZhStFKO5/qyYVFCgCkkRcwHwYDVR0jBBgwFoAUfc9m2WZh
StFKO5/qyYVFCgCkkRcwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC
AQEAj5qEsJveP8UGeOSTXdTfUyi8p9NoD74Gl76MnyGJVDOBb7ggDrYSnBJ+deQM
FhKQGQ3N0AGUcEi2on6yIuQWVh1sRlqoXFfLl5R9Bj3z+AGguxTEguTyTw+JqJDo
9pPMWSI5lK1XNk1hF3553rIeoBxmNg2dGwJiqxE5ApKL5wE7wwBt/3xAPzoa9FTo
zMAKml8gYaJhqfMBDGXUjvXh4n4Hwk2DHQqPx7J3OtKRwhuQtiSvMiKXaYeqKDla
7KiTi6F/ptIdS/nUcaoefbdtMriVIGdaV7hizM8AItObHNozmxXzRX14Qk97+ftI
QiATAcZYMOXe1kjNRFsoWcZiuQ==
-----END CERTIFICATE-----)";
    cert.issuer = "CN=ZMonitorTest";
    cert.subject = "CN=ZMonitorTest";
    cert.notBefore = QDateTime::currentDateTimeUtc().addDays(-1);
    cert.notAfter = QDateTime::currentDateTimeUtc().addDays(60);
    cert.installedAt = QDateTime::currentDateTimeUtc();

    auto idRes = mgr->install(cert);
    ASSERT_TRUE(idRes.isOk());

    auto fetched = mgr->getByName("tls-server");
    ASSERT_TRUE(fetched.isOk());
    ASSERT_TRUE(fetched.value().has_value());
    EXPECT_EQ(fetched.value()->name, QString("tls-server"));
}

TEST_F(CertificateManagerFixture, ExpiryDetection)
{
    CertificateRecord cert;
    cert.name = "soon";
    cert.type = CertificateType::TlsClient;
    // Valid self-signed test certificate (generated with openssl)
    cert.pem = R"(-----BEGIN CERTIFICATE-----
MIIDDzCCAfegAwIBAgIULdr6xt77NydwjMZGbRy3XuqRZaMwDQYJKoZIhvcNAQEL
BQAwFzEVMBMGA1UEAwwMWk1vbml0b3JUZXN0MB4XDTI1MTIwNDE0MDMzNVoXDTI2
MTIwNDE0MDMzNVowFzEVMBMGA1UEAwwMWk1vbml0b3JUZXN0MIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnPxKT7499qUlcDVX786DRlO/Bo+241bEC+2x
KQbBHvrlmYAQ2tAwiK0ec8Wy9YgKTzIqQcwevr2Z2LjaEMb9aHfECd+EKLv71b2Q
9ENkdx5GDVYH5o0rARV0edaOsIxMKETv3PuI7grpyhdAE942Z2wZ+yCNL8whSnEX
2KWbOjuyGQa/3+pKoJZTTXSB7q9gOL0gOoqDvNWMxNkim0J8oupXHaigD1bTpnwp
NWunGu0ti5UrR74ZK5fbtoKH5YSOx60suug9eQStnZ1FC8oAivYmDnHogmlBkc7H
+oLHq7VtAiHLxl5FfSvdVXds/F1Sr7KoJblyPgEUMYJR3BCBAQIDAQABo1MwUTAd
BgNVHQ4EFgQUfc9m2WZhStFKO5/qyYVFCgCkkRcwHwYDVR0jBBgwFoAUfc9m2WZh
StFKO5/qyYVFCgCkkRcwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC
AQEAj5qEsJveP8UGeOSTXdTfUyi8p9NoD74Gl76MnyGJVDOBb7ggDrYSnBJ+deQM
FhKQGQ3N0AGUcEi2on6yIuQWVh1sRlqoXFfLl5R9Bj3z+AGguxTEguTyTw+JqJDo
9pPMWSI5lK1XNk1hF3553rIeoBxmNg2dGwJiqxE5ApKL5wE7wwBt/3xAPzoa9FTo
zMAKml8gYaJhqfMBDGXUjvXh4n4Hwk2DHQqPx7J3OtKRwhuQtiSvMiKXaYeqKDla
7KiTi6F/ptIdS/nUcaoefbdtMriVIGdaV7hizM8AItObHNozmxXzRX14Qk97+ftI
QiATAcZYMOXe1kjNRFsoWcZiuQ==
-----END CERTIFICATE-----)";
    cert.issuer = "CN=ZMonitorTest";
    cert.subject = "CN=ZMonitorTest";
    cert.notBefore = QDateTime::currentDateTimeUtc().addDays(-1);
    cert.notAfter = QDateTime::currentDateTimeUtc().addDays(10);
    cert.installedAt = QDateTime::currentDateTimeUtc();
    ASSERT_TRUE(mgr->install(cert).isOk());
    auto fetched = mgr->getByName("soon");
    ASSERT_TRUE(fetched.isOk());
    ASSERT_TRUE(fetched.value().has_value());
    EXPECT_TRUE(mgr->isExpiringSoon(fetched.value().value(), 30));
}
