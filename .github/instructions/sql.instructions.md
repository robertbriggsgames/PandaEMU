---
applyTo: "sql/**/*.sql"
---

# PandaEMU SQL instructions

## Scope rules
- Distinguish schema changes from data/content changes.
- Do not alter schema unless explicitly asked.
- For normal tasks, prefer targeted content/data fixes.
- Do not rewrite large SQL sections when a narrow correction is sufficient.

## Required behavior
- Keep SQL changes scoped to the requested feature or bug.
- Preserve repository naming and formatting conventions.
- Avoid deleting data broadly unless explicitly requested.
- Avoid hidden compatibility hacks.

## Typical allowed work
- content fixes
- quest data adjustments
- creature / gameobject / script linkage fixes
- playerbot-related data changes that do not alter schema
- targeted migration fixes if explicitly scoped and safe

## High-risk areas
- schema migrations
- auth DB changes
- character DB changes
- world DB structure changes
- silent data rewrites affecting unrelated content

## Validation suggestions
After a SQL patch, suggest validation such as:
- import / execution sanity
- checking affected rows or records
- verifying linkage to scripts or entries
- in-game verification of the changed content
