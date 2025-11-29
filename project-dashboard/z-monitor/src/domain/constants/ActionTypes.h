/**
 * @file ActionTypes.h
 * @brief Constants for action types used in action logging and audit trails.
 * 
 * This file contains all action type constants used throughout the application
 * for logging user actions, configuration changes, and system events.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include <QString>

namespace zmon {
namespace ActionTypes {

// Authentication actions
constexpr const char* LOGIN = "LOGIN";
constexpr const char* LOGIN_FAILED = "LOGIN_FAILED";
constexpr const char* USER_LOGOUT = "USER_LOGOUT";
constexpr const char* AUTO_LOGOUT = "AUTO_LOGOUT";
constexpr const char* SESSION_EXPIRED = "SESSION_EXPIRED";

// Patient management actions
constexpr const char* ADMIT_PATIENT = "ADMIT_PATIENT";
constexpr const char* DISCHARGE_PATIENT = "DISCHARGE_PATIENT";
constexpr const char* TRANSFER_PATIENT = "TRANSFER_PATIENT";

// Settings actions
constexpr const char* CHANGE_SETTING = "CHANGE_SETTING";
constexpr const char* ADJUST_ALARM_THRESHOLD = "ADJUST_ALARM_THRESHOLD";
constexpr const char* RESET_SETTINGS = "RESET_SETTINGS";

// Notification actions
constexpr const char* CLEAR_NOTIFICATIONS = "CLEAR_NOTIFICATIONS";
constexpr const char* DISMISS_NOTIFICATION = "DISMISS_NOTIFICATION";

// Administrative actions
constexpr const char* VIEW_AUDIT_LOG = "VIEW_AUDIT_LOG";
constexpr const char* EXPORT_DATA = "EXPORT_DATA";
constexpr const char* ACCESS_DIAGNOSTICS = "ACCESS_DIAGNOSTICS";
constexpr const char* PROVISIONING_MODE_ENTERED = "PROVISIONING_MODE_ENTERED";

// View actions (optional, for analytics)
constexpr const char* VIEW_VITALS = "VIEW_VITALS";
constexpr const char* VIEW_ALARMS = "VIEW_ALARMS";
constexpr const char* VIEW_NOTIFICATIONS = "VIEW_NOTIFICATIONS";

} // namespace ActionTypes

namespace TargetTypes {

// Target types for action logging
constexpr const char* PATIENT = "PATIENT";
constexpr const char* SETTING = "SETTING";
constexpr const char* NOTIFICATION = "NOTIFICATION";
constexpr const char* AUTHENTICATION = "AUTHENTICATION";

} // namespace TargetTypes

namespace ActionResults {

// Action result values
constexpr const char* SUCCESS = "SUCCESS";
constexpr const char* FAILURE = "FAILURE";
constexpr const char* PARTIAL = "PARTIAL";

} // namespace ActionResults

namespace AdmissionSources {

// Admission source types
constexpr const char* MANUAL = "manual";
constexpr const char* BARCODE = "barcode";
constexpr const char* CENTRAL_STATION = "central_station";

/**
 * @brief Convert AdmissionSource enum to string constant.
 * 
 * @param source Admission source enum value
 * @return String constant for the admission source
 */
inline const char* toString(int source) {
    switch (source) {
        case 0:  // Manual
            return MANUAL;
        case 1:  // Barcode
            return BARCODE;
        case 2:  // CentralStation
            return CENTRAL_STATION;
        default:
            return MANUAL;
    }
}

} // namespace AdmissionSources

namespace EventTypes {

// Event types for admission logging
constexpr const char* ADMISSION = "admission";
constexpr const char* DISCHARGE = "discharge";
constexpr const char* TRANSFER = "transfer";

} // namespace EventTypes

namespace JsonKeys {

// JSON keys for action log details
constexpr const char* PATIENT_NAME = "patient_name";
constexpr const char* ADMISSION_METHOD = "admission_method";
constexpr const char* BED_LOCATION = "bed_location";
constexpr const char* DEVICE_LABEL = "device_label";
constexpr const char* TARGET_DEVICE = "target_device";
constexpr const char* SETTING_NAME = "setting_name";
constexpr const char* OLD_VALUE = "old_value";
constexpr const char* NEW_VALUE = "new_value";

} // namespace JsonKeys

} // namespace zmon

