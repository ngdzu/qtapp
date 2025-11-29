/**
 * @file LogFormatter.cpp
 * @brief Implementation of log formatting utilities.
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include <QStringList>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include "LogFormatter.h"

namespace zmon {
namespace Utils {

QString formatHuman(const LogEntry& entry) {
    QString result;
    
    // Timestamp: YYYY-MM-DD HH:MM:SS.mmm
    QString timestampStr = entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
    
    // Level: [LEVEL]
    QString levelStr = ::zmon::Utils::logLevelToString(entry.level);
    
    // Category: [category] (if present)
    QString categoryStr = entry.category.isEmpty() ? "" : QString("[%1]").arg(entry.category);
    
    // Message
    QString messageStr = entry.message;
    
    // Context: {key:value, ...} (if present)
    QString contextStr = "";
    if (!entry.context.isEmpty()) {
        QStringList contextPairs;
        for (auto it = entry.context.begin(); it != entry.context.end(); ++it) {
            contextPairs << QString("%1: %2").arg(it.key(), it.value().toString());
        }
        contextStr = QString(" {%1}").arg(contextPairs.join(", "));
    }
    
    // Source location: (file:line in function) (if present)
    QString locationStr = "";
    if (!entry.file.isEmpty() && entry.line > 0) {
        QString fileName = entry.file.split('/').last();  // Just filename, not full path
        locationStr = QString(" (%1:%2").arg(fileName).arg(entry.line);
        if (!entry.function.isEmpty()) {
            locationStr += QString(" in %1").arg(entry.function);
        }
        locationStr += ")";
    }
    
    // Assemble: timestamp [LEVEL] [category] message {context} (location)
    result = QString("%1 [%2] %3 %4%5%6")
        .arg(timestampStr)
        .arg(levelStr)
        .arg(categoryStr)
        .arg(messageStr)
        .arg(contextStr)
        .arg(locationStr);
    
    return result.trimmed();
}

QString formatJson(const LogEntry& entry) {
    QJsonObject json;
    
    // Timestamp: ISO 8601 format
    json["timestamp"] = entry.timestamp.toUTC().toString(Qt::ISODateWithMs);
    
    // Level: lowercase string
    json["level"] = ::zmon::Utils::logLevelToString(entry.level).toLower();
    
    // Category (if present)
    if (!entry.category.isEmpty()) {
        json["category"] = entry.category;
    }
    
    // Message
    json["message"] = entry.message;
    
    // Context: JSON object (if present)
    if (!entry.context.isEmpty()) {
        QJsonObject contextObj;
        for (auto it = entry.context.begin(); it != entry.context.end(); ++it) {
            QJsonValue value;
            QVariant var = it.value();
            
            // Convert QVariant to QJsonValue
            if (var.type() == QVariant::String) {
                value = var.toString();
            } else if (var.type() == QVariant::Int || var.type() == QVariant::LongLong) {
                value = var.toLongLong();
            } else if (var.type() == QVariant::Double) {
                value = var.toDouble();
            } else if (var.type() == QVariant::Bool) {
                value = var.toBool();
            } else {
                value = var.toString();  // Fallback: convert to string
            }
            
            contextObj[it.key()] = value;
        }
        json["context"] = contextObj;
    }
    
    // Thread ID (if present)
    if (!entry.threadId.isEmpty()) {
        json["threadId"] = entry.threadId;
    }
    
    // Source location (if present)
    if (!entry.file.isEmpty()) {
        json["file"] = entry.file;
    }
    if (entry.line > 0) {
        json["line"] = entry.line;
    }
    if (!entry.function.isEmpty()) {
        json["function"] = entry.function;
    }
    
    // Convert to compact JSON string
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Compact);
}

QString logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "CRITICAL";
        case LogLevel::Fatal:
            return "FATAL";
        default:
            return "UNKNOWN";
    }
}

QString escapeJsonString(const QString& str) {
    QString result;
    result.reserve(str.length() + 10);  // Pre-allocate for common case
    
    for (QChar ch : str) {
        if (ch == '"') {
            result += "\\\"";
        } else if (ch == '\\') {
            result += "\\\\";
        } else if (ch == '\n') {
            result += "\\n";
        } else if (ch == '\r') {
            result += "\\r";
        } else if (ch == '\t') {
            result += "\\t";
        } else if (ch.unicode() < 0x20) {
            // Control characters: \uXXXX
            result += QString("\\u%1").arg(static_cast<int>(ch.unicode()), 4, 16, QChar('0'));
        } else {
            result += ch;
        }
    }
    
    return result;
}

QString formatContextAsJson(const QVariantMap& context) {
    if (context.isEmpty()) {
        return "{}";
    }
    
    QJsonObject jsonObj;
    for (auto it = context.begin(); it != context.end(); ++it) {
        QJsonValue value;
        QVariant var = it.value();
        
        if (var.type() == QVariant::String) {
            value = var.toString();
        } else if (var.type() == QVariant::Int || var.type() == QVariant::LongLong) {
            value = var.toLongLong();
        } else if (var.type() == QVariant::Double) {
            value = var.toDouble();
        } else if (var.type() == QVariant::Bool) {
            value = var.toBool();
        } else {
            value = var.toString();
        }
        
        jsonObj[it.key()] = value;
    }
    
    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact);
}

} // namespace Utils
} // namespace zmon