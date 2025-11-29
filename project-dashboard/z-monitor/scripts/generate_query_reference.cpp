/**
 * @file generate_query_reference.cpp
 * @brief Standalone program to generate QUERY_REFERENCE.md from QueryCatalog.
 * 
 * This program can be compiled and run to generate the query reference documentation.
 * 
 * Usage:
 *   ./generate_query_reference > doc/QUERY_REFERENCE.md
 * 
 * @author Z Monitor Team
 * @date 2025-01-15
 */

#include "infrastructure/persistence/QueryRegistry.h"
#include <QCoreApplication>
#include <QTextStream>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QString doc = zmon::persistence::QueryCatalog::generateDocumentation();
    
    QTextStream out(stdout);
    out << doc;
    
    return 0;
}

