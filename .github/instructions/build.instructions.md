---
applyTo: "CMakeLists.txt,cmake/**,*.cmake,src/**/*.{c,cc,cpp,h,hpp},dep/**,contrib/**"
---

# PandaEMU build and tooling instructions

## Hard rules
- Do not upgrade dependencies, compiler baselines, or build strategy unless explicitly asked.
- Do not introduce cross-platform build changes casually.
- Do not change extractor or tooling behavior unless the task explicitly targets it.

## Preferred behavior
- Keep build changes minimal and local.
- Preserve current CMake structure and conventions.
- Treat map / vmap / mmap related tools as high-risk.
- If a build fix is needed, prefer fixing the smallest broken target or compatibility point.

## Extractor caution
- map, vmap, and mmap related changes are high-risk and often have runtime consequences beyond compilation.
- Do not propose speculative extractor rewrites.
