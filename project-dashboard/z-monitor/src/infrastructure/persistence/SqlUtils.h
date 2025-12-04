/**
 * @file SqlUtils.h
 * @brief SQL utility functions for parsing, cleaning, and processing SQL statements.
 *
 * This file contains reusable SQL utility functions that are used across the codebase,
 * particularly for parsing DDL files, splitting multi-statement SQL scripts, and handling
 * SQL comments.
 *
 * @author Z Monitor Team
 * @date 2025-12-04
 */

#pragma once

#include <QString>
#include <QStringList>

namespace zmon::sql
{
    /**
     * @brief Split multi-statement SQL script into individual statements.
     *
     * This function intelligently splits SQL on semicolons while:
     * - Respecting string literals (ignoring semicolons inside quotes)
     * - Handling inline SQL comments (-- comment)
     * - Working around DDL generator bug where commas appear inside comments
     * - Preserving SQL syntax by extracting commas from comments
     *
     * **DDL Generator Bug Workaround:**
     * The auto-generated DDL files have inline comments with trailing commas:
     * ```sql
     * id INTEGER PRIMARY KEY AUTOINCREMENT  -- Primary key,
     * ```
     * This function extracts the comma from the comment and places it correctly:
     * ```sql
     * id INTEGER PRIMARY KEY AUTOINCREMENT,
     * ```
     *
     * **Usage Example:**
     * ```cpp
     * QString multiStatementSql = loadFromFile("create_tables.sql");
     * QStringList statements = zmon::sql::splitSqlStatements(multiStatementSql);
     * for (const QString &stmt : statements) {
     *     QSqlQuery q(db);
     *     q.exec(stmt);
     * }
     * ```
     *
     * @param sql Multi-statement SQL script (may contain comments and multiple statements)
     * @return List of individual SQL statements, cleaned and ready for execution
     *
     * @note This function is stateless and thread-safe
     * @note Empty statements and comment-only lines are filtered out
     * @note Handles both double-quoted strings and SQL comments correctly
     *
     * @see stripSqlComments() for comment removal only
     * @see isSqlComment() for comment detection
     */
    QStringList splitSqlStatements(const QString &sql);

    /**
     * @brief Remove SQL inline comments from a single line.
     *
     * Removes `-- comment` style comments while preserving string literals.
     * Extracts trailing commas from within comments (DDL generator bug workaround).
     *
     * **Example:**
     * ```cpp
     * QString line = "name TEXT  -- Primary key,";
     * QString cleaned = stripSqlComments(line);
     * // Result: "name TEXT,"
     * ```
     *
     * @param line Single line of SQL (may contain inline comment)
     * @return Line with comment removed, comma preserved if present in comment
     *
     * @note Does not handle multi-line C-style comments
     * @note Respects string literals - will not remove -- inside quotes
     */
    QString stripSqlComments(const QString &line);

    /**
     * @brief Check if a trimmed SQL statement is a comment or empty.
     *
     * @param statement Trimmed SQL statement
     * @return true if statement is empty, whitespace-only, or a comment line
     *
     * @note Use this to filter out non-executable statements before exec()
     */
    bool isSqlComment(const QString &statement);

} // namespace zmon::sql
