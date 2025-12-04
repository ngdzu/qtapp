doc_id: DOC-COMP-033
title: Database Connection Pattern - Reader-Writer Segregation
version: v1.0
category: Component
subcategory: Infrastructure/Database
status: Published
owner: Database Team
reviewers:
  - Architecture Team
  - Infrastructure Team
last_reviewed: 2025-12-04
next_review: 2026-03-04
related_docs:
    - DOC-COMP-020 # SQLiteTelemetryRepository
    - DOC-COMP-021 # SQLiteAlarmRepository
    - DOC-ARCH-005 # Data Flow Architecture
    - DOC-COMP-022 # SQL Utilities
    - DOC-TROUBLE-001 # Database Test Cleanup Issue
related_tasks:
  - TASK-DB-001  # DatabaseManager implementation (completed)
related_requirements:
  - REQ-PERF-001  # Database concurrency requirements
  - REQ-PERF-005  # Database performance requirements
tags:
  - database
  - infrastructure
  - persistence
  - sqlite
  - concurrency
  - reader-writer
  - connection-management
diagram_files:
    - DOC-COMP-020_connection_pattern.mmd
    - DOC-COMP-020_connection_pattern.svg

# Database Connection Pattern - Reader-Writer Segregation

> **DOC-ID:** DOC-COMP-033 | **Version:** v1.0 | **Status:** Published | **Owner:** Database Team

## Overview

This document describes the three-connection pattern for database access in Z Monitor, following the centralized documentation and metadata standards in [DOC-GUIDE-004: Documentation Guidelines](../../.github/instructions/doc_guidelines.md).

...existing code...
