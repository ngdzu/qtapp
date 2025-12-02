#pragma once

/**
 * @file RepositoryTestFixture.h
 * @brief Extension of DatabaseTestFixture adding simple domain seeding helpers for
 *        repository integration tests.
 */

#include "tests/fixtures/DatabaseTestFixture.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include <memory>

namespace zmon::test
{
    class RepositoryTestFixture : public DatabaseTestFixture
    {
    protected:
        /**
         * @brief Create and admit a test patient aggregate.
         * @return Shared pointer to admitted PatientAggregate (nullptr on failure).
         */
        std::shared_ptr<PatientAggregate> createAdmittedPatient(
            const std::string &mrn = "MRN-TEST-001",
            const std::string &name = "Test Patient",
            int64_t dobMs = 0,
            const std::string &sex = "U",
            const std::vector<std::string> &allergies = {"None"},
            const std::string &bedId = "ICU-TEST",
            const std::string &unit = "ICU",
            const std::string &source = "fixture")
        {
            PatientIdentity identity(mrn, name, dobMs, sex, allergies);
            BedLocation bedLocation(bedId, unit);
            auto patient = std::make_shared<PatientAggregate>();
            auto result = patient->admit(identity, bedLocation, source);
            if (result.isError())
            {
                ADD_FAILURE() << "Failed to admit test patient: " << result.error().message;
                return nullptr;
            }
            return patient;
        }
    };
} // namespace zmon::test
