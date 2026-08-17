# Session 1 — Design

Architecture and approach only, not line-by-line implementation (that is `plan.md`).

## Context

- Single-header C++20 matrix library (`matrix.hpp`, 7,688 lines, CRTP mixin architecture — A1
  teardown is explicitly **not** this session's job).
- Two Critical heap-corruption bugs, both re-verified by ASan probes on the pre-fix tree
  (`.work/evidence/prefix_*.out/err`):
  - **C1** `crtp_shrink_to_size` (~3508): the per-row copy uses `the_rows_to_copy` as the column
    extent. The surrounding logic is already correct: `other` is allocated at the new size and
    zero-filled; the copy extents are `min`-derived; `zen.swap(other)` commits. Only the copy
    extent is wrong.
  - **C2** `flipdim` dim==2 (~4453): `swap_ranges` takes a column range (`col_begin/col_end`,
    `row()` elements, stride-`col()` iterator) as its first range but a **row** start
    (`row_begin(index_right)`, contiguous) as its third argument. The dim==1 branch (rows↔rows)
    is already correct.
- Iterator semantics (verified this session, `matrix.hpp` ~1925–1943): `col_begin(i)` starts at
  `dat+i` with stride `col()`; `col_end(i) = col_begin(i)+row()`; `row_begin(k) = dat+k·col()`.
  This is why the C2 bug over-reads/over-writes on non-square shapes (3×5: third range starts at
  `dat+4·5 = dat+20`, buffer holds 15) and silently scrambles square shapes (4×4: all accesses in
  bounds, content wrong).

## Goals / Non-Goals

**Goals**
1. Restore the documented contracts: `shrink_to_size` = documented copy + zero-pad/truncate
   (PRD §5 row 1); `flipdim(m,2)` = true left-right flip for all shapes (PRD §5 row 2).
2. Pin both with content-asserting regression tests + ASan-clean E01/E02 probes so the T1 pattern
   (green suite over untested paths) cannot recur on these functions.
3. Prove the tests catch the bugs: restoring either original bug line makes the new tests fail.

**Non-Goals** (out of scope — S3 or later)
- `fliplr`/`flipud` alias swap (C3; anchors 4491–4499 untouched).
- `flipdim` dim==1 behavior change (already correct; pinned, not changed).
- Any signature/semantics change beyond the documented contract; any CRTP refactor (A1).
- Docs/ReadMe edits (P4 — doc deltas expected: none; behavior matches documentation).

## Decisions

| # | Decision | Chosen | Alternatives considered | Why |
|---|---|---|---|---|
| D1 | C1 fix shape | 1-token: `the_rows_to_copy` → `the_cols_to_copy` in the `std::copy` | rewrite with `std::copy_n`/`span`; per-row `std::min` recompute | The review's smallest-safe-fix; contract invariant "diff limited to the two buggy lines + tests". The rest of the function already implements the documented semantics correctly. |
| D2 | C2 fix shape | 1-identifier: third arg `ans.row_begin( index_right )` → `ans.col_begin( index_right )` | rewrite dim==2 as row-reversal loop (mirroring dim==1) | `col_begin` gives the matching `row()`-element column range; the loop structure and the dim==1 branch stay untouched (minimum diff, minimum review surface). |
| D3 | Test location/style | New `tests/cases/shrink_to_size.hpp`, `tests/cases/flip.hpp`; Catch2 `TEST_CASE` + `REQUIRE`; exact expected values (integer-valued doubles, tol 1e-12) | extend an existing case file; shape-only asserts | Contract names the files; content assertions are the T1/T2 policy (P8). Style mirrors `ones.hpp`/`inverse.hpp`. |
| D4 | Probe design | Single `.work/probes/E01_E02.cc` with case selection (`all` default); ASan build per contract; per-case sub-invocations so one ASan abort doesn't mask the other reproductions | one probe per seed file | The contract's deterministic check compiles exactly this path and expects `PASS`. Pre-flight runs need per-case isolation (e01a and e02a both abort under ASan pre-fix). |
| D5 | "Tests catch the bug" evidence | Empirical bug-restoration: temporarily restore each buggy line → new cases FAIL → restore fix → green; record output | static argument that the asserts differ from buggy output | Deterministic evidence over narrative (AGENTS.md); answers the verifier's specific question. |
| D6 | Eval-seed status | `promoted` (probe exists + passes + permanent home in `tests/cases/`) | `live` | P9 clarification: "promoted" subsumes "live" per `eval_seed_cases.md` definitions; logged in the decision log. |

**ACD note (functional-thinking guardian pass):** `shrink_to_size` remains an in-place **Action**
(mutates the caller's matrix through `swap`); its internals (allocate + zero-fill + copy) stay
explicit Calculations over local data — the fix changes no boundary, only corrects an extent.
`flipdim` is a pure **Calculation** (copies input to `ans`, never mutates the caller's matrix);
mutation discipline is satisfied by the existing copy. New test code is explicit Data (literals) +
Calculation (assertions); no hidden Actions, no globals, no impurity creep. Guardian checks 1–6:
silent (clean).

## Risks / Trade-offs

- [Editing the wrong adjacent line] (dim==1 branch, `fliplr`/`flipud` at 4491–4499) → the diff is
  2 lines reviewed against the baseline commit; `flip.hpp` pins dim==1 behavior so an accidental
  edit fails the suite; the diff-audit check (`git diff --name-only`) plus the review axis
  "correctness" catches scope drift.
- [Shape-only tests (T1 anti-pattern)] → all test cases assert exact content on ragged values
  (contract `failure_modes_to_watch`: "test with ragged values, not 1.0 fills").
- [ASan probe built without `-DNDEBUG` masks the OOB (better_assert aborts first)] → the contract
  check command hard-codes the flags; pre-flight output recorded with the exact command.
- [Copy count fixed but a row-offset error remains] → e01b uses distinct values 1..30 so any
  row/col offset error changes the expected content; tests include both non-square directions.
- [Pre-fix probe evidence lost to later re-runs] → outputs captured in `.work/evidence/prefix_*`
  and cited in the handoff before the fix lands.
- Trade-off accepted: the 1-token fixes leave the surrounding slightly awkward structure
  (e.g., `size_type const the_rows_to_copy` next to `the_cols_to_copy` used by a row loop) as-is —
  readability is S3+/A1 territory; this session optimizes for review surface, not style.

## Migration Plan

Single-branch session on `phase-1/session-1` from baseline `83ea78d`; every intermediate state
keeps `make test` runnable. Rollback = `git reset --hard 83ea78d` (no data migration; no consumer
changes; no API change). No deployment steps (library repo).

## Open Questions

None blocking. (Q1–Q10 in `brainstorming.md` were resolved against the contract set at session
start; the only recorded refinement is D6, the eval-seed status clarification.)
