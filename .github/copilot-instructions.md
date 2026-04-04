# PandaEMU repository-wide Copilot instructions

This repository is a custom World of Warcraft emulator fork centered on PandaEMU / Mists of Pandaria 5.4.8 with playerbots.

## Primary goals
- Support safe work on playerbot features.
- Support dungeons, instance scripting, quests, content fixes, database work, and other important scripting.
- Preserve existing engine architecture and gameplay flow.

## Strict operating mode
- Always begin with analysis.
- Then propose a plan.
- Then make a small, scoped patch.
- Then propose validation steps.
- Never attempt a large multi-system implementation in one pass.

## Global prohibitions
- Do not refactor unless explicitly asked.
- Do not change files outside the requested task scope.
- Do not change database schema unless explicitly asked.
- Do not bypass or short-circuit existing combat, spell, aura, movement, session, packet, or map flows.
- Do not introduce hackfixes, temporary shortcuts, or silent fallbacks.
- Do not invent systems when an existing subsystem already owns the behavior.
- Do not change ELUNA/Lua for playerbot work unless explicitly requested.

## Required approach
- Prefer understanding the existing flow over rewriting it.
- Prefer minimal invasive changes over broad rewrites.
- Prefer existing helpers, managers, config, and script hooks over duplicating logic.
- Keep behavior aligned with current core conventions.
- If the task touches a danger zone, slow down and inspect call chains before editing.

## Danger zones
Treat these as high-risk areas. Avoid changes unless the task explicitly requires them.
- combat
- spell system
- aura system
- movement
- map / vmap / mmap
- packet handling
- auth / session
- DB migrations

## Scope discipline
Before proposing code, determine:
1. Which subsystem owns the requested behavior.
2. Whether the change belongs in `src/`, `modules/`, `sql/`, or scripts.
3. The smallest viable set of edits.

If the task target is unclear, prefer analysis and ask for clarification inside the response instead of making broad edits.

## Validation discipline
After proposing or making a patch, always suggest relevant validation such as:
- build / compile checks
- startup sanity checks
- config checks
- SQL import / execution checks
- targeted gameplay verification
- crash-risk or regression-risk checks

## Documentation support
When explicitly asked to produce documentation, generate concise repo-oriented documentation that:
- maps important directories to responsibilities
- identifies key classes, methods, managers, configs, and SQL locations
- distinguishes confirmed facts from assumptions
- avoids inventing architecture not evidenced by the repository
