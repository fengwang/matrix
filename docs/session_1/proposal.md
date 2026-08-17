# Session 1 — Proposal

Concise extraction from `brainstorming.md` (which in turn refines the blueprint set). Not a new
exploration.

## Motivation

- Two **Critical** heap-corruption bugs are reachable under ordinary use (review C1, C2; both
  re-verified by ASan probes on the pre-fix tree this session — `.work/evidence/prefix_*.out/err`):
  - C1: `shrink_to_size` copies `the_rows_to_copy` columns per row instead of `the_cols_to_copy`
    → heap OOB write (5×5→5×3) and silent content corruption (3×10→5×2).
  - C2: `flipdim(m,2)` swaps a column against a *row* (`swap_ranges` third arg `row_begin`)
    → heap OOB (3×5) and silent corruption (4×4).
- The suite is green **because** these paths are untested (T1). This session eliminates both bugs
  and pins them so the "green suite over untested paths" pattern cannot recur on these functions.

## Specific changes

1. `matrix.hpp` `crtp_shrink_to_size` (~3532): copy `the_cols_to_copy` per row (1 token).
2. `matrix.hpp` `flipdim` dim==2 branch (~4479): `swap_ranges` third argument →
   `ans.col_begin( index_right )` (1 identifier).
3. New `tests/cases/shrink_to_size.hpp` — content-asserting regression cases (E01 scenarios +
   adversarial shapes), registered in `tests/test.cc`.
4. New `tests/cases/flip.hpp` — content-asserting regression cases for `flipdim` dim 2 (E02) **and**
   dim 1 (regression pin for the untouched branch), registered in `tests/test.cc`.
5. `.work/probes/E01_E02.cc` — standalone ASan probes (pre-fix reproduction already recorded;
   post-fix must be clean + `PASS E01` / `PASS E02`).
6. `docs/eval_seed_cases.md`: E01/E02 status `seeded` → `promoted` (probe exists, passes, and has a
   permanent home in `tests/cases/`; clarification logged per P9 — subsumes the contract's "live").
7. `.work/handoff_session_1.md` from the template; decision log incl. pre-fix probe evidence.

Explicitly **not** changed: `fliplr`/`flipud` (4491–4499, S3), `flipdim` dim==1 branch, any
signature/semantics beyond the documented contract, ReadMe, examples, Makefile.

## Capabilities (contract between proposal and specifications)

### New capabilities

- **`regression-pinning`** — content-asserting, registered Catch2 cases for `shrink_to_size` and
  `flipdim` that fail if either original bug is present, plus the E01/E02 ASan probes registered in
  the eval-seed corpus.

### Modified capabilities

- **`shrink-to-size`** — the documented copy + zero-pad/truncate contract (in-code comment
  3515–3517; PRD §5 row 1) now actually holds: on shrink the top-left `min(rows)×min(cols)` block is
  preserved verbatim and dropped elements are gone; on growth the new region is zero.
- **`flipdim`** — `flipdim(m,2)` is a true left-right flip (`f[r][c] == m[r][col-1-c]`) for all
  shapes (PRD §5 row 2); `flipdim(m,1)` behavior is unchanged and pinned.

Each capability gets a spec file under `docs/session_1/specs/`.

## Impact

- **Code:** `matrix.hpp` (2 lines), `tests/test.cc` (2 includes), 2 new test files.
- **API:** none — no signature changes, no new public names, no behavior change beyond restoring
  the documented contract (sanctioned, PRD §5 rows 1–2).
- **Dependencies:** none (no new dependencies).
- **Systems:** `make test` suite gains 2 cases; eval corpus gains 2 promoted seeds; S3 downstream
  (alias swap) can now assume `flipdim(m,2)` is correct.
