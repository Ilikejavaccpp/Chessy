# AGENTS.md — Rules & Scope for AI Coding Agents

## 1. File Traversal & Token Conservation
- DO NOT run broad, recursive regex/grep searches across the entire repository.
- Rely on explicit file paths or targeted directory searches.
- Exclude build artifacts, binary files, generated outputs, `.git`, and external dependencies from context scans.

## 2. Unit Testing Rules
- DO NOT auto-generate sprawling test suites unless explicitly requested.
- Focus on micro-benchmarks or targeted integration checks.
- Keep unit test files concise; test individual pipeline functions rather than generating exhaustive edge-case matrix tests.

## 3. C Performance & Coding Standards
- Maintain cache-friendly design: avoid allocating dynamic memory (`malloc`/`free`) inside tight loop iterations or hot paths.
- Keep data layouts contiguous (Arena / Flat Buffer models).
- Do not add unnecessary C++ abstractions, wrapper classes, or heavy string-parsing headers.
- Always check return codes and ensure pointers are initialized before memory operations.

## 4. Git & Workflow Operations
- **NEVER use `git filter-branch`**. It is deprecated, slow, and spams output buffers.
- For removing sensitive/large paths from tracking before a push, use:
  - `git rm --cached -r <path>` for untracking files in current staging.
  - `git filter-repo --path <path> --invert-paths` for scrubbing full history.
- Always check `git status --porcelain` rather than piping raw `git status` output into heavy shell scripts.
- Validate builds locally before attempting git commits or pushes.
- Do not commit generated benchmark binaries or intermediate data files.
