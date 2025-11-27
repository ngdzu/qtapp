#!/usr/bin/env python3
"""
Create foundational knowledge document structure

Generates all placeholder files based on 00_FOUNDATIONAL_KNOWLEDGE_INDEX.md
"""

import os
from pathlib import Path

FOUNDATION_DIR = Path("project-dashboard/doc/foundation")

# Define all documents based on the index
DOCUMENTS = {
    "01_software_architecture_and_design_patterns": [
        ("01_domain_driven_design.md", "✅ Complete", "../../28_DOMAIN_DRIVEN_DESIGN.md"),
        ("02_dependency_injection.md", "✅ Complete", "../../13_DEPENDENCY_INJECTION.md"),
        ("03_repository_pattern.md", "✅ Complete", "../../30_DATABASE_ACCESS_STRATEGY.md (Section 3-4)"),
        ("04_data_transfer_objects.md", "✅ Complete", "../../31_DATA_TRANSFER_OBJECTS.md"),
        ("05_mvc_mvvm_patterns.md", "🔶 Partial", "../../02_ARCHITECTURE.md, ../../09_CLASS_DESIGNS.md"),
        ("06_state_machine_pattern.md", "✅ Complete", "../../05_STATE_MACHINES.md"),
        ("07_observer_pattern.md", "⏳ Planned", "To be created"),
        ("08_strategy_pattern.md", "⏳ Planned", "To be created"),
        ("09_factory_pattern.md", "⏳ Planned", "To be created"),
    ],
    "02_database_and_data_management": [
        ("01_database_normalization.md", "⏳ Planned", "To be created"),
        ("02_sqlite_wal_mode.md", "🔶 Partial", "../../30_DATABASE_ACCESS_STRATEGY.md (Section 5.1)"),
        ("03_database_indexing.md", "🔶 Partial", "../../10_DATABASE_DESIGN.md, ../../33_SCHEMA_MANAGEMENT.md"),
        ("04_transactions_acid.md", "⏳ Planned", "To be created"),
        ("05_time_series_data.md", "🔶 Partial", "../../10_DATABASE_DESIGN.md, ../../30_DATABASE_ACCESS_STRATEGY.md"),
        ("06_schema_versioning.md", "✅ Complete", "../../34_DATA_MIGRATION_WORKFLOW.md"),
        ("07_query_optimization.md", "🔶 Partial", "../../30_DATABASE_ACCESS_STRATEGY.md (Section 5)"),
        ("08_connection_pooling.md", "🔶 Partial", "../../30_DATABASE_ACCESS_STRATEGY.md (Section 4.1)"),
    ],
    "03_security_and_cryptography": [
        ("01_tls_ssl.md", "🔶 Partial", "../../06_SECURITY.md (Section 6.1), ../../15_CERTIFICATE_PROVISIONING.md"),
        ("02_encryption_at_rest.md", "🔶 Partial", "../../06_SECURITY.md (Section 2)"),
        ("03_digital_signatures.md", "🔶 Partial", "../../06_SECURITY.md (Section 6.3)"),
        ("04_authentication_authorization.md", "🔶 Partial", "../../06_SECURITY.md (Section 3-4)"),
        ("05_key_management.md", "🔶 Partial", "../../06_SECURITY.md (Section 2.3, 6.6)"),
        ("06_security_audit_logging.md", "✅ Complete", "../../06_SECURITY.md (Section 6.7), ../../21_LOGGING_STRATEGY.md"),
        ("07_input_validation.md", "⏳ Planned", "To be created"),
        ("08_secure_boot.md", "🔶 Partial", "../../06_SECURITY.md (Section 9)"),
    ],
    "04_concurrency_and_threading": [
        ("01_thread_safety.md", "🔶 Partial", "../../12_THREAD_MODEL.md (Section 8)"),
        ("02_thread_priorities.md", "✅ Complete", "../../12_THREAD_MODEL.md (Section 10)"),
        ("03_qt_event_loop.md", "🔶 Partial", "../../12_THREAD_MODEL.md (Section 8.1)"),
        ("04_producer_consumer.md", "⏳ Planned", "To be created"),
        ("05_realtime_constraints.md", "🔶 Partial", "../../12_THREAD_MODEL.md (Section 4)"),
        ("06_deadlock_prevention.md", "⏳ Planned", "To be created"),
    ],
    "05_memory_and_performance": [
        ("01_memory_management.md", "✅ Complete", "../../23_MEMORY_RESOURCE_MANAGEMENT.md"),
        ("02_memory_pools.md", "🔶 Partial", "../../23_MEMORY_RESOURCE_MANAGEMENT.md (Section 4)"),
        ("03_cache_optimization.md", "⏳ Planned", "To be created"),
        ("04_memory_profiling.md", "⏳ Planned", "To be created"),
        ("05_performance_profiling.md", "⏳ Planned", "To be created"),
    ],
    "06_error_handling_and_resilience": [
        ("01_error_handling.md", "✅ Complete", "../../20_ERROR_HANDLING_STRATEGY.md"),
        ("02_exception_safety.md", "🔶 Partial", "../../20_ERROR_HANDLING_STRATEGY.md (Section 2)"),
        ("03_circuit_breaker.md", "🔶 Partial", "../../06_SECURITY.md (Section 6.5)"),
        ("04_retry_backoff.md", "🔶 Partial", "../../06_SECURITY.md (Section 3.2), ../../20_ERROR_HANDLING_STRATEGY.md"),
        ("05_graceful_degradation.md", "⏳ Planned", "To be created"),
    ],
    "07_logging_and_observability": [
        ("01_logging_strategies.md", "✅ Complete", "../../21_LOGGING_STRATEGY.md"),
        ("02_metrics_telemetry.md", "🔶 Partial", "../../10_DATABASE_DESIGN.md (telemetry_metrics table)"),
        ("03_distributed_tracing.md", "⏳ Planned", "To be created"),
        ("04_health_checks.md", "⏳ Planned", "To be created"),
    ],
    "08_testing_strategies": [
        ("01_test_driven_development.md", "🔶 Partial", "../../18_TESTING_WORKFLOW.md"),
        ("02_test_doubles.md", "🔶 Partial", "../../18_TESTING_WORKFLOW.md, ../../13_DEPENDENCY_INJECTION.md"),
        ("03_property_based_testing.md", "⏳ Planned", "To be created"),
        ("04_performance_testing.md", "🔶 Partial", "../../18_TESTING_WORKFLOW.md (Section 4)"),
        ("05_integration_testing.md", "🔶 Partial", "../../18_TESTING_WORKFLOW.md (Section 3)"),
    ],
    "09_api_design_and_documentation": [
        ("01_api_design_principles.md", "⏳ Planned", "To be created"),
        ("02_api_versioning.md", "✅ Complete", "../../25_API_VERSIONING.md"),
        ("03_api_documentation.md", "✅ Complete", "../../26_API_DOCUMENTATION.md"),
        ("04_openapi_swagger.md", "🔶 Partial", "../../14_PROTOCOL_BUFFERS.md"),
        ("05_protocol_buffers.md", "🔶 Partial", "../../14_PROTOCOL_BUFFERS.md"),
    ],
    "10_qt_specific_knowledge": [
        ("01_qt_object_model.md", "⏳ Planned", "To be created"),
        ("02_qt_signals_slots.md", "✅ Complete", "../01_QT_SIGNALS_SLOTS.md (moved to root)"),
        ("03_qml_best_practices.md", "🔶 Partial", "Lessons 12-qtquick-qml-intro, 13-qtquick-controls"),
        ("04_qt_graphics_rendering.md", "🔶 Partial", "Lessons 14-graphicsview, 22-opengl-and-3d"),
        ("05_qt_model_view.md", "🔶 Partial", "Lessons 08-model-view-architecture, 09-custom-models"),
    ],
    "11_medical_device_standards": [
        ("01_iec_62304.md", "⏳ Planned", "To be created"),
        ("02_hl7_fhir.md", "⏳ Planned", "To be created"),
        ("03_iec_60601_alarms.md", "🔶 Partial", "../../04_ALARM_SYSTEM.md"),
        ("04_iec_62443_cybersecurity.md", "🔶 Partial", "../../06_SECURITY.md"),
    ],
    "12_devops_and_deployment": [
        ("01_ci_cd.md", "🔶 Partial", "../../18_TESTING_WORKFLOW.md (Section 7), ../../34_DATA_MIGRATION_WORKFLOW.md (Section 12)"),
        ("02_containerization.md", "✅ Complete", "Lesson Dockerfiles, DOCKER_SETUP.md"),
        ("03_configuration_management.md", "✅ Complete", "../../24_CONFIGURATION_MANAGEMENT.md"),
        ("04_semantic_versioning.md", "🔶 Partial", "../../25_API_VERSIONING.md"),
    ],
    "13_code_quality": [
        ("01_static_analysis.md", "🔶 Partial", "../../18_TESTING_WORKFLOW.md (Section 5)"),
        ("02_code_review.md", "🔶 Partial", "../../22_CODE_ORGANIZATION.md (Section 8)"),
        ("03_refactoring.md", "⏳ Planned", "To be created"),
        ("04_technical_debt.md", "⏳ Planned", "To be created"),
    ],
    "14_build_systems": [
        ("01_cmake_best_practices.md", "🔶 Partial", "Lesson CMakeLists.txt files"),
        ("02_package_management.md", "⏳ Planned", "To be created"),
        ("03_cross_compilation.md", "⏳ Planned", "To be created"),
    ],
}

def create_document(category_dir: Path, filename: str, title: str, status: str, reference: str):
    """Create a placeholder document."""
    filepath = category_dir / filename
    
    # Skip if already exists (manually created)
    if filepath.exists():
        print(f"⏭️  Skipping (exists): {filepath.relative_to(FOUNDATION_DIR)}")
        return
    
    content = f"""# {title}

> **📚 Foundational Knowledge**  
> This is a general software engineering concept used in Z Monitor's design.  
> See `../00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` for all foundational topics.

---

## Status: {status}

**Reference:** {reference}

This document provides foundational knowledge about {title.lower()}.

---

*Content to be populated.*
"""
    
    with open(filepath, 'w') as f:
        f.write(content)
    
    print(f"✅ Created: {filepath.relative_to(FOUNDATION_DIR)}")

def extract_title_from_filename(filename: str) -> str:
    """Extract title from filename."""
    name = filename.replace('.md', '').replace('_', ' ')
    # Remove number prefix
    parts = name.split(' ', 1)
    if len(parts) > 1 and parts[0].isdigit():
        name = parts[1]
    return name.title()

def main():
    print("📚 Creating Foundational Knowledge Document Structure\n")
    
    total_created = 0
    total_skipped = 0
    
    for category, documents in DOCUMENTS.items():
        category_dir = FOUNDATION_DIR / category
        category_dir.mkdir(parents=True, exist_ok=True)
        
        print(f"\n📁 {category.replace('_', ' ').title()}")
        print("━" * 60)
        
        for filename, status, reference in documents:
            title = extract_title_from_filename(filename)
            try:
                create_document(category_dir, filename, title, status, reference)
                total_created += 1
            except FileExistsError:
                total_skipped += 1
    
    print(f"\n" + "="*60)
    print(f"✅ Created {total_created} placeholder documents")
    print(f"⏭️  Skipped {total_skipped} existing documents")
    print(f"📊 Total: {total_created + total_skipped} files")
    print("="*60)

if __name__ == "__main__":
    main()

