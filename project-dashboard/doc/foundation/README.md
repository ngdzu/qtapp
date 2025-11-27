# Foundational Knowledge Documentation

This directory contains general software engineering knowledge, patterns, and best practices that underpin the Z Monitor project's design and architecture.

---

## 📋 Quick Start

**Start here:** `00_FOUNDATIONAL_KNOWLEDGE_INDEX.md`

The index document provides:
- Complete list of all 74 foundational topics
- Status of each topic (Complete/Partial/Planned)
- Cross-references to Z Monitor-specific implementations
- Priority recommendations for learning/documentation

---

## 📁 Directory Structure

The foundational knowledge is organized into 14 categories with 76 documents:

```
foundation/
├── 00_FOUNDATIONAL_KNOWLEDGE_INDEX.md (index of all topics)
│
├── 01_software_architecture_and_design_patterns/ (9 docs)
│   ├── 01_domain_driven_design.md
│   ├── 02_dependency_injection.md
│   ├── 03_repository_pattern.md
│   ├── 04_data_transfer_objects.md
│   ├── 05_mvc_mvvm_patterns.md
│   ├── 06_state_machine_pattern.md
│   ├── 07_observer_pattern.md
│   ├── 08_strategy_pattern.md
│   └── 09_factory_pattern.md
│
├── 02_database_and_data_management/ (8 docs)
│   ├── 01_database_normalization.md
│   ├── 02_sqlite_wal_mode.md
│   ├── 03_database_indexing.md
│   ├── 04_transactions_acid.md
│   ├── 05_time_series_data.md
│   ├── 06_schema_versioning.md
│   ├── 07_query_optimization.md
│   └── 08_connection_pooling.md
│
├── 03_security_and_cryptography/ (8 docs)
│   ├── 01_tls_ssl.md
│   ├── 02_encryption_at_rest.md
│   ├── 03_digital_signatures.md
│   ├── 04_authentication_authorization.md
│   ├── 05_key_management.md
│   ├── 06_security_audit_logging.md
│   ├── 07_input_validation.md
│   └── 08_secure_boot.md
│
├── 04_concurrency_and_threading/ (6 docs)
│   ├── 01_thread_safety.md
│   ├── 02_thread_priorities.md
│   ├── 03_qt_event_loop.md
│   ├── 04_producer_consumer.md
│   ├── 05_realtime_constraints.md
│   └── 06_deadlock_prevention.md
│
├── 05_memory_and_performance/ (5 docs)
│   ├── 01_memory_management.md
│   ├── 02_memory_pools.md
│   ├── 03_cache_optimization.md
│   ├── 04_memory_profiling.md
│   └── 05_performance_profiling.md
│
├── 06_error_handling_and_resilience/ (5 docs)
│   ├── 01_error_handling.md
│   ├── 02_exception_safety.md
│   ├── 03_circuit_breaker.md
│   ├── 04_retry_backoff.md
│   └── 05_graceful_degradation.md
│
├── 07_logging_and_observability/ (4 docs)
│   ├── 01_logging_strategies.md
│   ├── 02_metrics_telemetry.md
│   ├── 03_distributed_tracing.md
│   └── 04_health_checks.md
│
├── 08_testing_strategies/ (5 docs)
│   ├── 01_test_driven_development.md
│   ├── 02_test_doubles.md
│   ├── 03_property_based_testing.md
│   ├── 04_performance_testing.md
│   └── 05_integration_testing.md
│
├── 09_api_design_and_documentation/ (5 docs)
│   ├── 01_api_design_principles.md
│   ├── 02_api_versioning.md
│   ├── 03_api_documentation.md
│   ├── 04_openapi_swagger.md
│   └── 05_protocol_buffers.md
│
├── 10_qt_specific_knowledge/ (5 docs)
│   ├── 01_qt_object_model.md
│   ├── 02_qt_signals_slots.md ⭐ (comprehensive, ~800 lines)
│   ├── 03_qml_best_practices.md
│   ├── 04_qt_graphics_rendering.md
│   └── 05_qt_model_view.md
│
├── 11_medical_device_standards/ (4 docs)
│   ├── 01_iec_62304.md
│   ├── 02_hl7_fhir.md
│   ├── 03_iec_60601_alarms.md
│   └── 04_iec_62443_cybersecurity.md
│
├── 12_devops_and_deployment/ (4 docs)
│   ├── 01_ci_cd.md
│   ├── 02_containerization.md
│   ├── 03_configuration_management.md
│   └── 04_semantic_versioning.md
│
├── 13_code_quality/ (4 docs)
│   ├── 01_static_analysis.md
│   ├── 02_code_review.md
│   ├── 03_refactoring.md
│   └── 04_technical_debt.md
│
└── 14_build_systems/ (3 docs)
    ├── 01_cmake_best_practices.md
    ├── 02_package_management.md
    └── 03_cross_compilation.md
```

---

## 📚 How It Works

### **Structure:**

1. **Index Document:** `00_FOUNDATIONAL_KNOWLEDGE_INDEX.md`
   - Master list of all 74 topics
   - Status tracking (✅ Complete, 🔶 Partial, ⏳ Planned)
   - Cross-references to Z Monitor implementations
   - Priority recommendations

2. **Category Folders:** 14 numbered categories
   - Each contains related foundational topics
   - Documents numbered sequentially within category

3. **Topic Documents:** Individual markdown files
   - Placeholder files for all topics
   - Reference to Z Monitor-specific implementations
   - Ready to be populated with content

### **Document Format:**

Each foundational knowledge document follows this template:

```markdown
# Topic Name

> **📚 Foundational Knowledge**  
> This is a general software engineering concept used in Z Monitor's design.  
> See `../00_FOUNDATIONAL_KNOWLEDGE_INDEX.md` for all foundational topics.

---

## Status: ✅/🔶/⏳

**Document:** Path to this document
**Z Monitor Reference:** Path to project-specific implementation

This document provides foundational knowledge about [topic].

---

*Content follows...*
```

---

## 🎯 Purpose

### **Separation of Concerns:**

- **Foundational Knowledge** (`foundation/`) - General software engineering concepts
- **Z Monitor Implementation** (`doc/`) - Project-specific design and architecture

### **Benefits:**

✅ **Reusability** - Foundational docs can be referenced across multiple projects  
✅ **Clarity** - Separates "what is DDD?" from "how we implement DDD"  
✅ **Onboarding** - New developers learn concepts, then see implementations  
✅ **Documentation Quality** - General principles don't get mixed with project specifics

---

## 📖 Usage Guide

### **For New Developers:**

1. Start with `00_FOUNDATIONAL_KNOWLEDGE_INDEX.md`
2. Read ✅ Complete documents in relevant categories
3. Review Z Monitor-specific implementations in `doc/`

### **For Documentation Writers:**

1. Check index for topic status
2. Create/expand foundational documents (general principles)
3. Reference from Z Monitor-specific docs

### **For Code Reviewers:**

1. Reference foundational docs for patterns
2. Ensure code follows documented principles
3. Update docs when new patterns emerge

---

## 🔄 Workflow

### **Adding New Foundational Knowledge:**

1. Identify the topic and category
2. Add entry to `00_FOUNDATIONAL_KNOWLEDGE_INDEX.md`
3. Create numbered document in appropriate category folder
4. Write general principles (language/framework agnostic where possible)
5. Reference Z Monitor-specific implementation from main `doc/`

### **Updating Z Monitor Implementation:**

1. Update Z Monitor-specific doc in main `doc/` folder
2. Reference relevant foundation doc for general principles
3. Keep foundation doc focused on general knowledge

---

## 📊 Current Status

- **Total Topics:** 74
- **Complete:** 17 (23%)
- **Partial:** 29 (39%)
- **Planned:** 28 (38%)
- **Total Files:** 76 (including index + README)

### **High-Priority Topics (To Populate First):**

1. Qt Signals & Slots ✅ (DONE - 10/02_qt_signals_slots.md)
2. SQLite WAL Mode
3. Database Indexing
4. Thread Safety & Synchronization
5. Qt Event Loop & Threading
6. Input Validation
7. MVC/MVVM Patterns
8. Database Transactions & ACID

---

## 🔗 Related Documentation

- **Z Monitor Architecture:** `../02_ARCHITECTURE.md`
- **Z Monitor Design:** `../09_CLASS_DESIGNS.md`
- **Project Structure:** `../27_PROJECT_STRUCTURE.md`

---

*This directory provides the theoretical foundation for Z Monitor's design decisions. All design choices in the main documentation should reference relevant foundational principles from this directory.*

