---
applyTo: "src/**/*.{h,hpp,c,cc,cpp},modules/**/*.{h,hpp,c,cc,cpp}"
---

# PandaEMU core instructions

## Core mindset
You are modifying a live emulator core. Preserve engine flow first, patch second.

## Hard rules
- Do not refactor for style or cleanup unless explicitly asked.
- Do not move logic across subsystems unless explicitly asked.
- Do not bypass existing managers, update loops, handlers, or dispatch pipelines.
- Do not add speculative abstractions.
- Do not widen scope from a local bug to a systemic rewrite.

## Required workflow
1. Identify the owning subsystem.
2. Read nearby code and entry points before editing.
3. Follow existing call chains.
4. Patch the smallest correct location.
5. Keep naming, style, and control flow consistent with surrounding code.
6. Recommend focused verification after the change.

## Sensitive subsystems
Use extra caution with:
- combat
- spell casting and spell resolution
- aura application, stacking, removal, recalculation
- movement generators and pathing
- packet handlers and opcode routing
- auth / session lifecycle
- map, vmap, mmap, and extractor-adjacent code

## Combat / spell / aura constraints
- Never bypass existing spell or aura processing.
- Never hardcode outcomes that should be produced by normal combat or spell logic.
- Never duplicate damage, proc, aura, or target selection logic if a canonical path already exists.
- If behavior appears broken, trace the real source rather than patching symptoms.

## Movement constraints
- Respect existing movement generators, state transitions, and map validity checks.
- Do not inject shortcut teleport-like behavior unless the task explicitly requires it.
- Do not fake movement completion when pathing or navigation is the real issue.

## Session / packet constraints
- Preserve packet flow, authentication state, and session ownership.
- Avoid changes that silently relax validation or error handling.

## Modules boundary
- Do not move core logic into `modules/` unless explicitly directed.
- Do not modify `modules/` when the task is clearly core-only.
