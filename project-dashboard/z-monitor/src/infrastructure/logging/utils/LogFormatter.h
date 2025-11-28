/**
 * @file LogFormatter.h
 * @brief Shared formatting utilities for log entries.
 * 
 * This file contains utility functions for formatting LogEntry objects
 * into human-readable or JSON strings. Used by CustomBackend and other
 * logging backends that need formatting capabilities.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#pragma once

#include "../LogEntry.h"
#include <QString>

namespace ZMonitor {
namespace Infrastructure {
namespace Logging {
namespace Utils {

/**
 * @brief Formats a log entry as a human-readable string.
 * 
 * Produces a formatted string suitable for human reading, with timestamp,
 * level, category, message, and optional context fields.
 * 
 * Example output:
 * ```
 * 2025-01-15 10:30:45.123 [INFO] [network] Connection established {deviceId: "DEV-001"}
 * ```
 * 
 * @param entry Log entry to format.
 * @return Formatted string.
 */
QString formatHuman(const LogEntry& entry);

/**
 * @brief Formats a log entry as a JSON string.
 * 
 * Produces a JSON object with all log entry fields, suitable for
 * machine parsing and log aggregation systems.
 * 
 * Example output:
 * ```json
 * {"timestamp":"2025-01-15T10:30:45.123Z","level":"info","category":"network","message":"Connection established","context":{"deviceId":"DEV-001"},"threadId":"0x1234","file":"NetworkManager.cpp","line":42,"function":"connectToServer"}
 * ```
 * 
 * @param entry Log entry to format.
 * @return JSON-formatted string.
 */
QString formatJson(const LogEntry& entry);

/**
 * @brief Converts a LogLevel enum to a string representation.
 * 
 * @param level Log level to convert.
 * @return String representation (e.g., "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL", "FATAL").
 */
QString logLevelToString(LogLevel level);

/**
 * @brief Escapes a string for JSON output.
 * 
 * Escapes special characters (quotes, backslashes, newlines, etc.)
 * to make the string safe for JSON embedding.
 * 
 * @param str String to escape.
 * @return Escaped string.
 */
QString escapeJsonString(const QString& str);

/**
 * @brief Formats a QVariantMap as a JSON object string.
 * 
 * Converts a QVariantMap to a JSON object representation.
 * 
 * @param context Context map to format.
 * @return JSON object string (e.g., '{"key1":"value1","key2":123}').
 */
QString formatContextAsJson(const QVariantMap& context);

} // namespace Utils
} // namespace Logging
} // namespace Infrastructure
} // namespace ZMonitor

