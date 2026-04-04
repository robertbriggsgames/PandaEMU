---
applyTo: "src/**/*.{h,hpp,c,cc,cpp},modules/**/*.{h,hpp,c,cc,cpp},sql/**/*.{sql},*.conf,**/playerbots.conf"
---

# PandaEMU playerbot instructions

Playerbot support in this repository is important but sensitive. Work conservatively.

## Hard rules
- Do not use ELUNA/Lua as an implementation path for playerbot tasks unless explicitly requested.
- Do not bypass normal player, combat, spell, aura, movement, session, or map logic just to make bots "work".
- Do not fake state transitions that a real player path would normally produce.
- Do not introduce broad rewrites to unrelated AI systems.
- Do not change bot database schema unless explicitly asked.

## Preferred approach
- Treat bots as participants in existing game systems, not as exceptions outside them.
- Reuse canonical player-facing and server-facing flows where appropriate.
- Prefer local, auditable fixes over "helper" layers that hide incorrect behavior.
- Keep configuration, DB access, and runtime behavior separated cleanly.

## Playerbot task workflow
1. Identify whether the issue is bootstrap, config, DB, AI, combat, movement, spell, map, or scripting.
2. Confirm whether the correct place is core, module, config, or SQL.
3. Patch the smallest correct ownership point.
4. Recommend verification steps specific to bots.

## Typical playerbot areas
- login / bootstrap
- world entry
- bot DB access and config
- movement and pathing
- target selection
- combat participation
- spell usage
- dungeon / quest / content interactions

## Things to avoid
- hidden hardcoded target rules
- temporary toggles left behind in production code
- bypassing map or pathing checks to suppress movement failures
- bypassing aura or spell checks to force casts
- bundling playerbot changes with unrelated core cleanup

## Validation hints
Prefer validation suggestions such as:
- clean startup with playerbot config enabled
- bot DB connection / config sanity
- bot login / world entry sanity
- movement sanity on known map areas
- combat spell and aura sanity
- dungeon or quest scenario reproduction
