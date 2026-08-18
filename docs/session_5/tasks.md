# Session 5 — Tasks

TDD per task: failing check first (red), minimal change (green), targeted check,
self-critique, commit. Pre-fix red states are evidenced in `.work/evidence/s5_prefix.log`.
C11's red is **structural** (grep 3→0 + TSan + the pre-fix P7/P8 evidence) — the
suite case is an invariant pin, green both sides (interview Q2).

## T1 — `rand-regression-tests` (E14)
- **Red:** N/A by design (invariant pin; P9 proves determinism/range hold pre-fix).
- **Change:** new `tests/cases/rand.hpp` (design §5) + `#include "./cases/rand.hpp"`
  in `tests/test.cc` (after `proj.hpp`).
- **Targeted check:** `make test` → 74 cases, all pass (pre-fix engine).
- **Self-critique:** local bools (Catch quirk); int pin documents the all-zeros
  consequence; noexcept static_assert; no NaN asserts (fast-math).

## T2 — `rand-engine` (C11)
- **Red (structural, pre-fix verified):** `grep -cE 'srand\(|std::rand\(' matrix.hpp`
  = 3; pre-fix E15-class evidence: global-state data race (libc-internal, TSan
  blind — documented) + per-call-site seed-0 correlation (P8).
- **Green:** body swap to `std::mt19937` + `std::uniform_real_distribution<T>`
  (design §1) + `noexcept` removals (5323/5355/5361/5366) + comment `(0, 1)` → `[0, 1)`.
- **Targeted check:** grep = 0; `make test` green (74); E14 probe half green;
  TSan probe clean post-fix; P0 probe re-run (explicit-seed determinism still holds;
  value stream now differs — R-07 expected).
- **Self-critique:** seed-0 expression bit-identical to pre-fix; int/complex
  consequences documented; no new includes; `random`/aliases untouched.

## T3 — `core-count-guard` (C12)
- **Red:** N/A executable (cannot force `hardware_concurrency()==0` here) —
  structural red: `grep -c 'total_cores < 1'` = 0, `grep -c 'parallel_size < 1'` = 0.
- **Green:** the two clamps (design §2); 279 untouched.
- **Targeted check:** greps = 1 each; `make test` green; `git diff` shows only the
  four lines.
- **Self-critique:** clamp fires only on 0; 4121 behavior-neutral (P5); 279
  verbatim.

## T4 — `save-png-boundary` (S2-finding / R3-slice)
- **Red (executable, pre-fix verified):** E15 probe → **SIGSEGV, exit 139** (P7).
- **Green:** `if ( ! fp ) return;` after `fopen` (3183) + stray `;;` removed (3190).
- **Targeted check:** E15 probe → exit 0 + positive-control PNG exists; `make test`
  green; `make example` PNG outputs rebuild (closeout).
- **Self-critique:** happy path byte-identical (empty-statement removal only);
  `save_as_png` untouched; `load_binary` NOT touched (watch item).

## T5 — `ndebug-policy-doc` (C7)
- **Red:** N/A (doc capability — no code).
- **Change:** draft delta text authored (design §4 = spec text); no file outside
  `docs/session_5/**` touched.
- **Targeted check:** `grep -c 'NDEBUG' docs/session_5/design.md` ≥ 1; delta text
  present in spec + design + (later) handoff.
- **Self-critique:** mechanism claims line-checked against 57–61/`print_assertion`;
  I/O-boundary precedents named (load_npy S2, save_png S5); S4 guards classified
  as control flow.

## T6 — Full checks + adversarial verification
- `make test` (full, 74 cases); `make example` (stdout delta vs pre-fix recorded in
  `.work/evidence/s5_example_delta.txt`; `git checkout -- images/`); verbatim
  combined probe `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5 .work/probes/E14_E15.cc && .work/probe_s5` → `PASS`.
- Adversarial verification per the contract's `adversarial_cases` (seeds 0/1/large;
  float vs double; int; threads; complex-T documented; noexcept; 3469 call site;
  R-07 examples) → `docs/session_5/adversarial_verification.md`.

## T7 — Sharded review (risk medium → 4 shards × 6 axes)
- Per `docs/prompts/sharded_review.md`; findings triaged; High/Critical fixed with
  regression evidence → `docs/session_5/sharded_review.md`.

## T8 — Closeout
- `docs/eval_seed_cases.md`: E14 `seeded` → `promoted` (suite case + probe refs);
  E15 stays `live` (probe-only by design) with the post-fix result.
- `docs/risk_register.md`: S5 watch items (4121 discrepancy, R-07 stream change,
  complex-T compile impact, noexcept-chain extension, `load_binary` adjacent
  warning, residual seed-0 correlation).
- `.work/handoff_session_5.md`: state snapshot, decision log (D1–D10 +
  discrepancies/warnings), evidence map, eval-seed status, S6 hand-off notes
  (C7 delta text; alias noexcept state).
- Final full checks after any review fixes.
