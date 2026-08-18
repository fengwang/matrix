# Session 4 — Execution Contract

Source: `docs/session_4_contract.yaml` (authoritative) + refinements resolved in
`interview.md`/`brainstorming.md` (C-11 boundary, C8 shape). This is the executable
contract: what will change, what will not, and what evidence proves it.

## In scope (exactly)

| # | File | Lines/region | Change |
|---|---|---|---|
| 1 | `matrix.hpp` | 7769-7792 (`mean`/`variance`/`standard_deviation`) | type-class dispatcher: complex legacy verbatim; real → double (double fast path; int/float via `astype<double>()`); `size≤1` stddev → `double{}` (real) / `value_type{}` (complex) |
| 2 | `matrix.hpp` | 6754-6755 (`conv` "same" asserts) | `rb > 1` → `rb >= 1` (first), `rb > 1` → `cb >= 1` (second — the copy-paste fix) |
| 3 | `matrix.hpp` | 6486-6488 (`gauss_jordan_elimination` assert) | `row < col` → `row > 0 && col > 0`, message rewritten |
| 4 | `matrix.hpp` | 5766-5784 (`cholesky_decomposition`) | `void` → `bool`; diagonal-step guard `sum <= value_type(0) → false` (non-complex) before the `sqrt`; `return true` at completion |
| 5 | `tests/test.cc` | include block | +3 lines: `cholesky.hpp` (after `ceil.hpp`), `conv_same.hpp` (after `cos.hpp`), `rref.hpp` (after `rint.hpp`) |
| 6 | `tests/cases/mean.hpp` | file tail | +int/float/double content cases + `static_assert` types (existing double case untouched) |
| 7 | `tests/cases/conv_same.hpp` | new | E11 content + rb/cb directions + valid regression |
| 8 | `tests/cases/rref.hpp` | new | E12 content + singular square + wide regression |
| 9 | `tests/cases/cholesky.hpp` | new | E13 five inputs |
| 10 | `docs/eval_seed_cases.md` | E10–E13 rows | `seeded` → `promoted` |
| 11 | `docs/risk_register.md` | tail | S4 closeout watch items |
| 12 | `docs/session_4/**`, `.work/**` | — | phase docs, probes, evidence, handoff |

All within the contract's `allowed_files`. Nothing else.

## Out of scope (do not touch)
- `conv` full/valid/slice arithmetic; the `conv` swap heuristic.
- `gauss_jordan_elimination` algorithm body; the `row > col` OOB (documented + ASan-pinned,
  not repaired — a future session's algorithm-body decision); the `std::optional` return;
  the `1e-10` threshold.
- Cholesky arithmetic; zero-fill; complex PD semantics.
- `sum`'s int accumulator; the n−1 formula; population-vs-sample; `reduce`; `operator-`;
  `astype`; any `noexcept`/`constexpr` semantics change.
- ReadMe.md (S6), examples, `images/` (checkout policy), Makefile, production dependencies.
- No new public API beyond the contract's four sanctioned lines + the four test files.

## Evidence contract (done condition, verifiable)
1. `.work/evidence/prefix_p0.log` — pre-fix evidence (done): int mean `unsigned long`/1,
   int variance/stddev compile errors, conv SIGABRT@6755, rref SIGABRT@6486, row>col ASan
   heap OOB (pre-existing), cholesky `-nan`.
2. `git diff` per task touches only the table's lines (line-count audit in the commit).
3. `make test` green after every task (case count 69 → 73); `./test_test` all pass.
4. Contract deterministic check verbatim: `g++ -std=c++20 -DPARALLEL -O1 -o
   .work/probe_s4 .work/probes/E10_E13.cc && .work/probe_s4` → `PASS`.
5. Post-fix p3 ASan rerun identical in substance to pre-fix (row>col unchanged).
6. `make example` stdout identical to the S3 baseline.
7. `docs/session_4/sharded_review.md` — 4 shards × 6 axes, all Critical/High resolved with
   regression evidence.
8. `docs/session_4/adversarial_verification.md` — every `adversarial_cases` entry executed
   and passing.
9. `.work/handoff_session_4.md` — state snapshot, decision log, warnings, eval seeds.
10. `docs/eval_seed_cases.md` E10–E13 promoted; `docs/risk_register.md` S4 watch items.

## Stop conditions (project contract)
Stop and surface to the user (do not widen scope): a finding requiring an unsanctioned
behavior change; a contract line that cannot be satisfied as written (SPEC_GAP/AMBIGUITY);
an environment failure blocking a required check (record ENVIRONMENT evidence first).
