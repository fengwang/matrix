# Session 5 — Execution Contract

Source: `docs/session_5_contract.yaml` (authoritative) + refinements resolved in
`interview.md`/`brainstorming.md` (grep refinements Q6, 4121 discrepancy Q5,
noexcept-chain extension Q3, TSan limitation Q7.5). This is the executable contract:
what will change, what will not, and what evidence proves it.

## In scope (exactly)

| # | File | Lines/region | Change |
|---|------|--------------|--------|
| 1 | `matrix.hpp` | 5320–5337 (`rand` body + declaration) | per-call local `std::mt19937` + `std::uniform_real_distribution<T>(0.0, 1.0)`; seed 0 keeps the exact pre-fix `time + &ans` mix; `noexcept` dropped; comment `(0, 1)` → `[0, 1)` |
| 2 | `matrix.hpp` | 5355, 5361, 5366 (`rand_like`/`random_like`/`randn_like`) | `noexcept` dropped (same-defect extension, interview Q3) |
| 3 | `matrix.hpp` | 1152–1153 (`reduce`) | `const` dropped from `total_cores` + `if ( total_cores < 1 ) total_cores = 1;` |
| 4 | `matrix.hpp` | 4121–4122 (`reduce_impl_private`) | `if ( parallel_size < 1 ) parallel_size = 1;` (behavior-neutral clamp; 4121 was already short-circuit-safe — discrepancy logged) |
| 5 | `matrix.hpp` | 3183–3184 (`save_png`) | `if ( ! fp ) return;` after the `fopen` (silent no-op, `noexcept` kept) |
| 6 | `matrix.hpp` | 3190 | stray `;;` → `;` (R3-slice) |
| 7 | `tests/test.cc` | 59 (include block) | +1 line: `#include "./cases/rand.hpp"` (after `proj.hpp`) |
| 8 | `tests/cases/rand.hpp` | new | E14 case (design §5): determinism, seed inequality, [0,1) double+float, int all-zeros, type pins, noexcept pin |
| 9 | `docs/eval_seed_cases.md` | E14 row | `seeded` → `promoted` (suite case + probe refs); E15 stays `live` with post-fix result |
| 10 | `docs/risk_register.md` | tail | S5 watch items (4121 discrepancy, R-07 stream change, complex-T compile impact, noexcept-chain extension, `load_binary` adjacent warning, residual seed-0 correlation) |
| 11 | `docs/session_5/**`, `.work/**` | — | phase docs, probes, evidence, adversarial + review reports, handoff |

All within the contract's `allowed_files`. Nothing else.

## Out of scope (do not touch)
- The alias *bodies* and *renames* (`random`, `random_like`, `randn_like` → S6).
- `better_assert` macro, `debug_mode`, any ReadMe.md edit (C7 is authored text only).
- `load_binary` (S2's adjacent `fopen` warning — risk-register watch item only).
- Thread-pool sizing heuristics, the `mat.size() < 32` threshold, line 279's guard.
- Examples, Makefile, `images/` (checkout policy), production dependencies.
- Complex-T `rand` support (documented consequence, no in-repo consumers).
- No new public API beyond the sanctioned changes; no behavior change beyond the
  contract's four findings.

## Evidence contract (done condition, verifiable)

1. `.work/evidence/s5_baseline_test.log` — pre-fix green baseline (done): 73 cases /
   49,217,182 assertions; live seeds E01–E13 PASS.
2. `.work/evidence/s5_prefix.log` — pre-fix evidence (done): srand grep = 3; guard
   greps = 0; ltrace seed trace (per-call-site correlation); E15 SIGSEGV exit 139;
   TSan pre-fix clean (libc limitation); pre-fix value streams.
3. `git diff` per task touches only the table's lines (line-count audit per commit).
4. C11: `grep -cE 'srand\(|std::rand\(' matrix.hpp` = 0 post-fix (refined grep,
   interview Q6.1).
5. C12: `grep -c 'total_cores < 1'` = 1 and `grep -c 'parallel_size < 1'` = 1
   post-fix; line 279 unchanged.
6. png: `grep -cF 'if ( ! fp )' matrix.hpp` ≥ 1 post-fix.
7. `make test` green after every task (73 → 74 cases); `./test_test` all pass.
8. Verbatim combined probe: `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5
   .work/probes/E14_E15.cc && .work/probe_s5` → `PASS` (post-fix).
9. TSan post-fix: clean (no user-code races).
10. `make example` stdout delta recorded (`.work/evidence/s5_example_delta.txt`,
    R-07 — values differ where seeds are explicit); `git checkout -- images/`.
11. `docs/session_5/sharded_review.md` — 4 shards × 6 axes; all Critical/High
    resolved with regression evidence.
12. `docs/session_5/adversarial_verification.md` — every contract
    `adversarial_cases` entry executed (or documented, e.g. complex-T compile
    attempt) and passing.
13. `.work/handoff_session_5.md` — state snapshot, decision log, warnings,
    eval-seed status, S6 notes (C7 delta text; alias noexcept state).
14. `docs/eval_seed_cases.md` E14 promoted; `docs/risk_register.md` S5 watch items.

## Stop conditions (project contract §1.3)
Stop and surface to the user (do not widen scope): a finding requiring an
unsanctioned behavior change; a contract line that cannot be satisfied as written
(SPEC_GAP/AMBIGUITY); an environment failure blocking a required check (record
ENVIRONMENT evidence first).
