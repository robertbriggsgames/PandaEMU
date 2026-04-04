---
applyTo: "src/server/scripts/**/*.{h,hpp,c,cc,cpp},src/scripts/**/*.{h,hpp,c,cc,cpp},modules/**/*.{h,hpp,c,cc,cpp},sql/**/*.sql"
---

# PandaEMU content scripting instructions

Use these rules for dungeons, instances, quests, gossip, events, and other content scripting.

## Hard rules
- Preserve existing encounter flow and script ownership.
- Do not patch content problems by changing unrelated global systems unless explicitly required.
- Do not invent mechanics or quest logic not supported by repository data and code.
- Do not mix unrelated content fixes in one patch.

## Preferred approach
- Identify the exact content entry, script, SQL, and spawn/data linkage first.
- Prefer fixing the owning script or data record over adding global exceptions.
- Keep event state, boss progression, quest flags, and gossip flow consistent with existing patterns.

## Instance / dungeon caution
- Do not bypass encounter state machines.
- Do not hardcode boss progression if there is an existing instance script state model.
- Avoid quick fixes that mask reset, evade, or phase problems.

## Quest caution
- Respect quest state transitions, required conditions, and script linkage.
- Verify whether the root cause is script code, SQL content, template data, or event wiring.
