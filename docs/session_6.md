# Session 6 — Modernization: fast `fft`/`ifft`, `fftshift` fix, alias retirement, ReadMe sweep (P1, C13, A2, A3)

> Story outline v1. Refine at start (narrow/clarify only), policy P9.
> Machine-readable authority: `docs/session_6_contract.yaml`. Law: `docs/project_contract.md`.
> **Terminal session.** Depends on S1–S5 (doc deltas + fixed behavior to document). Must complete last.

## Objective

Deliver the modernization: (1) a fast separable radix-2 `fft`/`ifft` with the naive DFT kept as the non-power-of-2 fallback and the missing `1/(R·C)` normalization added to `ifft` (P1); (2) probe-first fix of the odd-dimension `fftshift`/`ifftshift` remap pinned to NumPy convention (C13); (3) retirement of the duplicate alias names (A2); (4) the A3 verification pass + namespace-hygiene policy note (no code change expected); (5) the ReadMe sweep that lands every doc delta from S2–S5 plus the FFT/alias content.

## Story

The review's stage 5 + the API-hygiene slice, plus the deep-research-grounded direction (NumPy conventions as semantic authority). **P1**, verified this turn against the current code: `fft`/`ifft` are **correct O(n⁴) naive DFTs**, not the "no-op stub" the review describes — the real gaps are performance (O(n⁴) vs O(n² log n)) and the **missing `1/(R·C)` normalization in `ifft`**, which makes `ifft(fft(x)) == R·C·x` instead of `x` (NumPy normalizes the inverse). The fix: separable 1D radix-2 FFT (columns then rows, `std::complex<T>` via the existing `fft_private::add_complex` promotion) for power-of-2 dimensions; the existing naive loops become the *fallback* for non-power-of-2 dimensions (the "documented fallback" decision from the PRD — no new math, the old code is the safety net). Differential tests against the naive DFT (kept as an oracle in the test) prove equivalence; the round-trip E16 pins the new normalization. **C13**, verified this turn: `fftshift`/`ifftshift` apply a **swap-based** remap (not the review's described index remap); for even `n` it equals NumPy's roll, for odd `n=5` it yields row order (3,4,2,0,1) where NumPy's `fftshift` (roll by `(n+1)/2 = 3`) yields (2,3,4,0,1) — probe first (P5), then replace the swap block with a circular roll by `(n+1)/2` per axis; the fused transform+shift design (here `fftshift` = shift∘`fft`) is **kept** and documented as a deliberate deviation from NumPy's pure reindex. **A2**: retire `random`, `random_like`, `pinverse`, `svd_inverse`, free `det(m)` (verified: zero consumers in `tests/`/`examples/` except `examples/cases/0013_prefix.hpp:3` using `feng::random` and ReadMe §1277/§117 examples); the pseudoinverse core moves to `matrix_details::pinv_core` behind the canonical `pinv`. **A3**: verified unsupported in the current file — no `feng::elem::` calls, no `elem` namespace; the pass is a pre-flight grep + a hygiene policy note in the handoff (no code change unless the grep finds a real instance).

## In scope

- P1: `fft` (anchor 6313) / `ifft` (anchor 6446) — separable radix-2 (power-of-2 dims) + naive-DFT fallback (non-power-of-2); `ifft` gains `1/(R·C)`; `fft_private`/`ifft_private::add_complex` promotion kept (float input ⇒ `complex<float>` output, as today); naive loops retained as `fft_private::naive_fft`/`ifft` fallback (or equivalent internal name).
- C13: `fftshift` (6349) / `ifftshift` (6480) — circular roll by `(n+1)/2` per axis, replacing the swap block; probe-first with recorded pre-fix output (n=4 match, n=5 mismatch vs NumPy).
- A2: delete `random` (5262), `random_like` (5267), `pinverse` (5226), `svd_inverse` (5216, core → `matrix_details::pinv_core`), free `det(m)` (4319); update `examples/cases/0013_prefix.hpp:3` to `feng::rand`.
- A3: pre-flight verification grep (`feng::elem`, `namespace elem`, qualified cross-namespace calls — expected: none) + policy note in the handoff. **No code change** unless the grep finds a real instance (then: fix only that instance, report it).
- ReadMe sweep (**sole ReadMe editor**): apply S2–S5 doc deltas verbatim; update §117 & §1277 `random` examples to `rand`; add FFT section (radix-2 + fallback + `ifft` normalization + `fftshift` convention & fused-design note); add the alias-retirement table (retired name → canonical); land the C7 NDEBUG policy text from S5.
- Tests: new `tests/cases/fft.hpp` (differential vs naive oracle; E16 round-trip; E17; `fftshift` even/odd vs pinned NumPy values); registration.

## Out of scope

- Any redesign of `fftshift` to NumPy's pure-reindex signature (fused design kept — documented); changes to `add_complex`'s type promotion (float ⇒ `complex<float>` preserved — P7); DLPack/interop (parked, R-17); CRTP restructure (deferred, decision C-03); `svd`/`eigen`/other numerics; the `TODO:` comments at 1997/2368/2431/2439/3884/3894 (unrelated); `examples/` value outputs (print-only; `make example` is a compile check).
- Editing any other session's findings (S1–S5 are closed and green — if one is not, **stop** and report).

## Deliveries

1. Fast FFT + normalized `ifft` + `fftshift` roll fix in `matrix.hpp`.
2. A2 retirements + `pinv_core` move + `0013_prefix.hpp` update.
3. `tests/cases/fft.hpp` + registration; A3 policy note.
4. ReadMe fully updated (all deltas + FFT section + retirement table + NDEBUG policy).
5. E16/E17 probes live; handoff closing the project (all 22 findings traced: fixed or deferred).

## Context budget map (~60K of 128K — do not exceed)

| Read | How much | Why |
|---|---|---|
| `AGENTS.md`, `docs/project_contract.md` §3–§7 | ~2.5K | law |
| `docs/prd.md` §5 rows 15–17, §6 S6, §7 | ~2K | authorization |
| `docs/opencode_sharded_review.md` §P1 §C13 §A2 §A3 | ~3K | findings (verified corrections are in this doc's story above — trust those over the review's descriptions) |
| `matrix.hpp` regions: 5210–5280 (pinv core + aliases), 4310–4330 (free det), 6290–6500 (entire FFT block: `fft_private`, `fft`, `fftshift`, `ifft_private`, `ifft`, `ifftshift`) | ~20K | the code |
| `examples/cases/0013_prefix.hpp` (≤10 lines) | ~0.5K | the `feng::random` consumer |
| `.work/handoff_session_{2,3,4,5}.md` — doc-delta sections only | ~6K | the exact ReadMe wording to land |
| `ReadMe.md` — targeted sections only: §100–130 (random example), §760–790 (det), §1040–1070 (load_npy), §1270–1285, §1740–1800 (random prose), §2220–2250 (pinv) | ~6K | the sweep targets |
| `docs/eval_seed_cases.md` E16–E17 | ~0.5K | probe specs |
| **Do NOT read:** rest of `matrix.hpp`, ReadMe in full, research docs in full, other handoffs in full | — | budget |

## Deep-research references

- Report 6 §"Proposed public API" (line 243): the NumPy-convention API surface (fft naming, normalization, shift) — the convention authority for P1/C13. Section only.
- P3500R0 §"Interoperability with std::mdspan and std::linalg" (line 274): context for where FFT/linalg semantics sit in the standardization direction. Section only.

## Pre-flight (mandatory)

1. `git status` clean; confirm S1–S5 all merged/green (`make test`; E02/E09 live) — if not, stop.
2. Re-anchor every region by grep; log drift.
3. **A3 verification pass** (record in handoff): `grep -c "feng::elem\|namespace elem" matrix.hpp` → expected 0; scan for qualified cross-namespace calls → expected none. If any found: log to risk register, fix only that instance.
4. **C13 probe-first**: run the pre-fix `fftshift` on 4×4 and 5×5 real inputs; compare against NumPy `np.fft.fftshift` roll semantics (expected: n=4 equal, n=5 mismatch (3,4,2,0,1) vs (2,3,4,0,1)). Record.
5. **P1 baseline**: record the current `ifft(fft(x)) == R·C·x` behavior (pre-normalization) so E16 shows the change.

## Exit criteria

1. `make test` green (suite + `fft.hpp`); `make example` green (compiles with `feng::rand` in 0013).
2. E16: 8×8 delta ⇒ all ones within 1e-9 **and** `‖ifft(fft(x)) − x‖∞ < 1e-9` (fixed 8×8 input; pre-fix baseline `R·C·x` recorded first); E17: `fftshift`/`ifftshift` row orders match the pinned NumPy values (3×1 ⇒ both `(1,2,0)`; 4×1 ⇒ both rotate by 2); the differential-vs-oracle test lives in `fft.hpp` (8×8 radix-2 path and 6×8 fallback path, `< 1e-9` — oracle frozen per R-18).
3. `fftshift`/`ifftshift` match the pinned NumPy roll for n=4 and n=5 (asserted in `fft.hpp`).
4. A2: `grep -c "random\b" matrix.hpp` → 0 (except `random_device`/prose comments if any — document); `pinv` still works (E06 still passes); 0013 compiles.
5. ReadMe sweep complete: every S2–S5 delta landed (diff against the delta lists in the handoffs), FFT section + retirement table + NDEBUG policy present.
6. `git diff --name-only HEAD` ⊆ {`matrix.hpp`, `tests/test.cc`, `tests/cases/fft.hpp`, `examples/cases/0013_prefix.hpp`, `ReadMe.md`, `.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`}.
7. Sharded review + adversarial verifier PASS (verifier focus: radix-2 bit-reversal correctness vs the oracle on 2×4/4×2/1×8/8×1 shapes; fallback path selected for 6×8; `ifft` normalization applied exactly once; A2 leaving no dangling references; ReadMe claims all true against the final code).
8. Project-closing handoff: findings table (22 rows: fixed-in-session / deferred with ref), all eval seeds' live status.

## Risk and routing

- Risk level **medium** (largest new-logic session but: the naive DFT stays as oracle and fallback, A2 is deletion of verified-consumer-less names, ReadMe is text). Routing: **worker_plus_reviewers**.
- Failure modes to watch: bit-reversal/stride bugs (the oracle differential is the pin — if it fails, the fix is wrong, not the oracle); normalization applied to `fft` by mistake (it goes on `ifft` only); `fftshift` even-n behavior changed (it must stay bit-identical for even dims — regression case); A2 deleting a name the ReadMe sweep still references (sweep and deletion in the same session, same diff); float `complex<float>` tolerance (use 1e-4, not 1e-9, in float differential cases).

## Handoff requirements

State snapshot (compiler, checks run/not run); decision log (A3 grep result; C13 pre-fix probe outputs; P1 baseline `ifft∘fft` demo; contract refinements); eval seeds E16/E17 live; namespace-hygiene policy note (A3); **project-closing findings table** (22 rows: fixed/deferred + session refs); final `make test` + `make example` evidence.

## Contract

`docs/session_6_contract.yaml` — read before pre-flight; authority for scope, invariants, checks.
