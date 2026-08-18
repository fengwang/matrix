# Session 4 — Tasks

TDD per task: failing spec-derived test first (red), minimal change (green), targeted check,
self-critique, commit. Red states are pre-fix-verified (probe log) — "red" here is concrete:
abort, value failure, or compile error.

## T1 — `stat-promotion` (C8)
- **Red:** extend `tests/cases/mean.hpp` with the int promotion case (mean 1.5 etc.) +
  `static_assert` double types. Pre-fix red = compile error (int variance/stddev don't
  compile — p0b/p0c evidence) + mean value failure (`1 != 1.5`).
- **Green:** rewrite the three statistic bodies (design §1, type-class dispatcher).
- **Targeted check:** `make test` (fast: 9.5s build) → `./test_test`.
- **Self-critique:** int 1×2/2×2/1×1 + float + double content; static_asserts; complex path
  untouched; `constexpr` intact.

## T2 — `conv-same-kernel` (C9)
- **Red:** new `tests/cases/conv_same.hpp` case (a) 1×1 kernel. Pre-fix red = SIGABRT in the
  suite build (asserts live — p1 evidence). (b)–(d) pass pre-fix (regression net).
- **Green:** the two assert conditions (design §2).
- **Targeted check:** `make test` + `./test_test`.
- **Self-critique:** rb==1/cb==1 both directions; valid regression; no other conv line.

## T3 — `rref-domain` (C10)
- **Red:** new `tests/cases/rref.hpp` case (a) square diag. Pre-fix red = SIGABRT in the
  suite build (p2 evidence). (b) singular + (c) wide pass pre-fix (regression net).
- **Green:** the precondition line (design §3).
- **Targeted check:** `make test` + `./test_test`; example 0020 stdout unchanged
  (`make example` delta check at closeout).
- **Self-critique:** square/wide/singular; row>col NOT in the suite (UB); message style.

## T4 — `cholesky-guard` (P2b)
- **Red:** new `tests/cases/cholesky.hpp` with `bool ok = feng::cholesky_decomposition(...)`.
  Pre-fix red = compile error (`void` return assigned to `bool`).
- **Green:** `bool` return + diagonal guard (design §4).
- **Targeted check:** `make test` + `./test_test`.
- **Self-critique:** all five adversarial inputs; no NaN assertions needed; complex path
  compiles (no in-repo complex callers — template still instantiable for double).

## T5 — Deterministic probe + seeds
- Build/run the contract's deterministic check: `g++ -std=c++20 -DPARALLEL -O1 -o
  .work/probe_s4 .work/probes/E10_E13.cc && .work/probe_s4` → `PASS` (post-fix, asserts
  live: E11/E12 no longer abort; E10 exact values; E13 all five cases).
- Post-fix ASan rerun of p3 (row>col) → compare with pre-fix log (identical).
- `docs/eval_seed_cases.md`: E10–E13 `seeded` → `promoted` with the probe + test file refs.

## T6 — Full checks + sharded review + adversarial verification
- `make test` (full), `make example` (stdout delta vs S3 baseline; `git checkout -- images/`
  policy), `make clean`-safe state.
- Sharded review per `docs/prompts/sharded_review.md` (risk medium → 4 shards × 6 axes),
  findings triaged; High/Critical fixed with regression evidence.
- Adversarial verification per `docs/prompts/adversarial_verifier.md` against
  `docs/session_4_contract.yaml` `adversarial_cases` (E10–E13, conv directions, rref
  square/singular/wide/row>col, cholesky five inputs) with fresh-context simulation
  (subagent environment constraint — S1 record — keeps all review work in-process).
- `docs/risk_register.md` watch items; `.work/handoff_session_4.md` per
  `docs/templates/handoff.md`; done-condition check (every contract line verified against
  evidence).

## Commit points (S1–S3 convention)
1. `S4 pre-flight: phase docs, probes, pre-fix evidence` (before any product edit).
2. `S4 task 1: C8 statistics return double for integer/float matrices`
3. `S4 task 2: C9 conv same-mode accepts 1x1 kernels`
4. `S4 task 3: C10 rref accepts square systems (precondition relaxation)`
5. `S4 task 4: P2 cholesky_decomposition returns bool with PD guard`
6. `S4 closeout: E10-E13 probes live, eval seeds promoted, review + adversarial records`
