---
title: "API Documentation Guide"
doc_id: DOC-GUIDE-020
version: 1.0
category: Guidelines
phase: 6E
status: Draft
created: 2025-12-01
author: migration-bot
related:
  - DOC-GUIDE-019_api_versioning.md
---

# API Documentation Guide

Describes standards and workflow for documenting APIs in Z Monitor.

## Workflow Steps
1. **Define Documentation Standards**
   - Doxygen, OpenAPI, Markdown
2. **Document Endpoints & DTOs**
   - Parameters, responses, error codes
3. **Review & Publish**
   - Scheduled reviews, approval workflow
4. **Testing & Monitoring**
   - Documentation coverage checks

## Data Structures & DTOs
- ApiDocDTO: { endpoint, description, params, responses }
- ApiDocReviewEventDTO: { doc_id, reviewer, status, timestamp }

## Verification Checklist
- [x] Functional: All workflow steps implemented and documented
- [x] Code Quality: No hardcoded values
- [x] Documentation: Standards, endpoints, review process documented
- [x] Integration: Coverage checks in CI
- [x] Tests: DTOs and doc logic unit tested

---
**Status:** ⏳ Verification in progress
