/**
 * @file SqlUtils.cpp
 * @brief Implementation of SQL utility functions.
 */

#include "SqlUtils.h"

namespace zmon::sql
{
    QString stripSqlComments(const QString &line)
    {
        QString processedLine = line;
        bool inString = false;

        // Find the comment start position
        for (int i = 0; i < line.size() - 1; ++i)
        {
            if (line[i] == '"')
            {
                inString = !inString;
            }
            else if (!inString && line[i] == '-' && line[i + 1] == '-')
            {
                // Found comment start at position i
                // Check if comment ends with a comma (DDL generator bug workaround)
                QString comment = line.mid(i);
                bool hasCommaInComment = comment.trimmed().endsWith(',');

                // Get the part before the comment
                QString beforeComment = line.left(i).trimmed();

                if (hasCommaInComment && !beforeComment.isEmpty())
                {
                    // Add comma after the SQL, not in the comment
                    processedLine = beforeComment + ",";
                }
                else
                {
                    // Just remove the comment
                    processedLine = beforeComment;
                }
                break;
            }
        }

        return processedLine;
    }

    bool isSqlComment(const QString &statement)
    {
        const QString trimmed = statement.trimmed();
        return trimmed.isEmpty() || trimmed.startsWith("--");
    }

    QStringList splitSqlStatements(const QString &sql)
    {
        // Step 1: Process inline comments (-- comment until end of line)
        // Extract any trailing comma from within the comment and place it before the comment
        QString cleanedSql;
        QStringList lines = sql.split('\n');

        for (const QString &line : lines)
        {
            QString processedLine = stripSqlComments(line);
            cleanedSql.append(processedLine + "\n");
        }

        // Step 2: Split on semicolons not inside strings
        QStringList statements;
        QString currentStatement;
        bool inString = false;
        const QChar quote('"');

        for (int i = 0; i < cleanedSql.size(); ++i)
        {
            QChar c = cleanedSql.at(i);

            if (c == quote)
            {
                inString = !inString;
                currentStatement.append(c);
            }
            else if (c == ';' && !inString)
            {
                // End of statement
                statements << currentStatement;
                currentStatement.clear();
            }
            else
            {
                currentStatement.append(c);
            }
        }

        // Add the last statement if not empty
        if (!currentStatement.trimmed().isEmpty())
        {
            statements << currentStatement;
        }

        return statements;
    }

} // namespace zmon::sql
