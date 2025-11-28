/**
 * @file TestPatientAggregate.cpp
 * @brief Unit tests for PatientAggregate domain aggregate.
 * 
 * This file contains unit tests for the PatientAggregate class, verifying
 * business rules, state transitions, and domain logic.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <cassert>
#include <iostream>
#include "domain/monitoring/PatientAggregate.h"
#include "domain/admission/PatientIdentity.h"
#include "domain/admission/BedLocation.h"
#include "domain/monitoring/VitalRecord.h"

using namespace ZMonitor::Domain::Monitoring;
using namespace ZMonitor::Domain::Admission;

/**
 * @brief Test patient admission workflow.
 */
void testAdmission() {
    PatientAggregate patient;
    
    // Initially not admitted
    assert(patient.getAdmissionState() == AdmissionState::NotAdmitted);
    assert(!patient.isAdmitted());
    
    // Admit a patient
    PatientIdentity identity("MRN-12345", "John Doe", 946684800000, "M");
    BedLocation bed("ICU-4B", "ICU");
    
    bool admitted = patient.admit(identity, bed, "manual");
    assert(admitted);
    assert(patient.isAdmitted());
    assert(patient.getAdmissionState() == AdmissionState::Admitted);
    assert(patient.getPatientMrn() == "MRN-12345");
    assert(patient.getPatientIdentity().name == "John Doe");
    
    std::cout << "✓ Test: Patient admission succeeded\n";
}

/**
 * @brief Test that only one patient can be admitted at a time.
 */
void testSinglePatientRule() {
    PatientAggregate patient;
    
    PatientIdentity identity1("MRN-12345", "John Doe", 946684800000, "M");
    BedLocation bed1("ICU-4B", "ICU");
    
    PatientIdentity identity2("MRN-67890", "Jane Smith", 946684800000, "F");
    BedLocation bed2("ICU-4C", "ICU");
    
    // Admit first patient
    bool admitted1 = patient.admit(identity1, bed1, "manual");
    assert(admitted1);
    
    // Try to admit second patient (should fail)
    bool admitted2 = patient.admit(identity2, bed2, "manual");
    assert(!admitted2);
    assert(patient.getPatientMrn() == "MRN-12345");  // First patient still admitted
    
    std::cout << "✓ Test: Single patient rule enforced\n";
}

/**
 * @brief Test patient discharge workflow.
 */
void testDischarge() {
    PatientAggregate patient;
    
    PatientIdentity identity("MRN-12345", "John Doe", 946684800000, "M");
    BedLocation bed("ICU-4B", "ICU");
    
    // Admit patient
    patient.admit(identity, bed, "manual");
    assert(patient.isAdmitted());
    
    // Discharge patient
    bool discharged = patient.discharge();
    assert(discharged);
    assert(!patient.isAdmitted());
    assert(patient.getAdmissionState() == AdmissionState::Discharged);
    assert(patient.getDischargedAt() > 0);
    
    std::cout << "✓ Test: Patient discharge succeeded\n";
}

/**
 * @brief Test that vitals can only be recorded for admitted patients.
 */
void testVitalsRequireAdmission() {
    PatientAggregate patient;
    
    // Try to record vital without admission (should fail)
    VitalRecord vital("HR", 72.0, 1700000000000, 100, "MRN-12345", "DEV-001");
    bool updated = patient.updateVitals(vital);
    assert(!updated);
    
    // Admit patient
    PatientIdentity identity("MRN-12345", "John Doe", 946684800000, "M");
    BedLocation bed("ICU-4B", "ICU");
    patient.admit(identity, bed, "manual");
    
    // Now record vital (should succeed)
    updated = patient.updateVitals(vital);
    assert(updated);
    
    // Verify vital was recorded
    auto recentVitals = patient.getRecentVitals(1);
    assert(recentVitals.size() == 1);
    assert(recentVitals[0].vitalType == "HR");
    assert(recentVitals[0].value == 72.0);
    
    std::cout << "✓ Test: Vitals require admission rule enforced\n";
}

/**
 * @brief Test that vitals must match patient MRN.
 */
void testVitalsMatchPatientMrn() {
    PatientAggregate patient;
    
    PatientIdentity identity("MRN-12345", "John Doe", 946684800000, "M");
    BedLocation bed("ICU-4B", "ICU");
    patient.admit(identity, bed, "manual");
    
    // Try to record vital with wrong MRN (should fail)
    VitalRecord wrongVital("HR", 72.0, 1700000000000, 100, "MRN-99999", "DEV-001");
    bool updated = patient.updateVitals(wrongVital);
    assert(!updated);
    
    // Record vital with correct MRN (should succeed)
    VitalRecord correctVital("HR", 72.0, 1700000000000, 100, "MRN-12345", "DEV-001");
    updated = patient.updateVitals(correctVital);
    assert(updated);
    
    std::cout << "✓ Test: Vitals must match patient MRN rule enforced\n";
}

/**
 * @brief Main test runner.
 */
int main() {
    std::cout << "Running PatientAggregate unit tests...\n\n";
    
    try {
        testAdmission();
        testSinglePatientRule();
        testDischarge();
        testVitalsRequireAdmission();
        testVitalsMatchPatientMrn();
        
        std::cout << "\n✓ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << "\n";
        return 1;
    }
}

