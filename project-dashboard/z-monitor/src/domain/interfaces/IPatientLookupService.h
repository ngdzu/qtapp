/**
 * @file IPatientLookupService.h
 * @brief Interface for patient lookup operations.
 *
 * Provides methods to lookup patients by identifiers and attributes.
 */
#pragma once

#include "domain/common/Result.h"
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include <string>
#include <vector>

namespace zmon
{

    class IPatientLookupService
    {
    public:
        virtual ~IPatientLookupService() = default;

        /**
         * @brief Lookup patient by MRN.
         * @param mrn Medical Record Number
         * @return Result<PatientAggregate> - Patient aggregate if found
         */
        virtual Result<PatientAggregate> getByMrn(const std::string &mrn) const = 0;

        /**
         * @brief Search patients by name.
         * @param name Full or partial name
         * @return Result<std::vector<PatientIdentity>> - Matching patient identities
         */
        virtual Result<std::vector<PatientIdentity>> searchByName(const std::string &name) const = 0;
    };

} // namespace zmon
