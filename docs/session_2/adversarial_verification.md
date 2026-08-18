# Session 2 — Adversarial verification

Process note: no subagent tool in this environment — the fresh-context pass was run in-session
with the verifier's inputs only (contract, project contract, diff vs `ad6fa79`, evidence).
Mindset followed per `docs/prompts/adversarial_verifier.md`: assume the completion claim is
false; think like an attacker; trace data flow across boundaries.

## Attempted falsifications (new attacks beyond the suite + 18-case probe)

New crafted inputs (`.work/probes/extra_attacks.cc`, ASan `-DNDEBUG`, log
`.work/evidence/probe_adv_attacks.log`), run against the committed fix (`88c3740`;
`git diff 88c3740 -- matrix.hpp` empty → probe binaries valid):

| Attack | Input | Expected | Result |
|---|---|---|---|
| A1 | v2, 12-byte total (data prefix 12 → 0 header bytes), `header_length` 0 and 1 | false (V4 / bound) | ok ×2 |
| A2 | v1, `header_length` = 0 exactly (inclusive bound passes) | false (empty header, V4) | ok |
| A3 | 1 MB header of `{` + junk, no descr | clean false, no OOM beyond file | ok |
| A4 | directory passed as file name | false (empty/failed read → size gate) | ok |
| A5 | otherwise-valid 2×3 `<f4` file into `matrix<double>` after a valid 2×3 load | false + prior state/content untouched | ok |
| A6 | shape (2³⁰, 2³⁰), 16-byte file | false **before** resize; no 2⁶⁰ allocation; state 0×0 | ok |
| A7 | shape (2⁴⁰, 2²⁴) — product wraps `size_t` (the third hazard) | false in the overflow-checked multiply | ok |

`PASS EXTRA-ATTACKS`, exit 0, no ASan report.

## Call-stack traces

- **UP (callers):** the public contract is `bool load_npy(…) noexcept` + "false ⇒ matrix
  unchanged on every rejection path". All `false` returns occur before `zen.resize`
  (verified: the only `resize`/`reshape`/`copy` sit after the last bound); the sole path where
  `zen` may already be mutated is `bad_alloc` from `resize` itself — the spec (R-V9) promises
  false/throw-free/UB-free there, not state-unchanged (that is `resize`'s own exception-safety
  property, pre-existing, out of scope). Callers checking the bool are unaffected by the
  change; callers that ignored it pre-fix now get strictly more rejections (the sanctioned
  row-18 change).
- **DOWN (callees):** `zen.resize(row, col)` receives `row, col ≥ 1` with `row*col ≤
  (buffer bytes)/sizeof(T)` — no overflow possible in `resize`'s internal arithmetic (the
  product was already overflow-checked upstream). `zen.reshape(col, row)` (fortran path) gets
  the same element count. `std::copy_n` range `[data_offset, data_offset+payload)` is provably
  inside the buffer (R-V8 bound) and exactly `row*col` values inside `zen` (resize precedes it;
  byte count = payload by construction).
- **State across operations:** repeated loads on one matrix (valid→rejected, rejected→valid)
  covered by A5 and suite case 5 (valid fixture loaded first, rejected loads in between, state
  re-checked). No persistent state is touched (no statics, no globals — the body allocates only
  `buffer`, `header`, and the matrix storage).

## Check-list verdicts

1. **Acceptance criteria:** all four contract `acceptance` rows covered by suite + probe (3B/11B
   suite+probe; 0xFFFFFFFF suite+probe; missing-shape suite+probe; `<f4`-into-double suite+probe;
   4 fixtures suite; ASan release probe exit 0; `grep -c 'return false'` = 12 > 6).
2. **Invariants:** every row of the contract `invariants` table has a suite or probe pin
   (missing file → suite case 1, **including the assert-enabled build** — the pre-fix SIGABRT
   proves the D12 change was load-bearing).
3. **Blast radius:** `git diff --name-only ad6fa79` ⊆ allowed set (verified: 0 files outside);
   `matrix.hpp` diff = exactly one hunk inside the `load_npy` body (`@@ -2509 +2509`);
   `tests/cases/load_npy.hpp` diff = 0 deleted lines (append-only, existing case byte-identical).
4. **Tests fail if core behavior breaks:** bug-restoration (task 5.2) disabled the `header_length`
   bound → the 0xFFFFFFFF case segfaulted (exit 139); pre-fix red runs show all 5 cases failing
   (SIGABRT×2, SIGSEGV, 2 false-positive `true`s). The dtype gate is pinned independently of the
   payload bound by the exact-size `>f8` case. **One gap found and closed:** bad-magic ≥12B and
   zero-dim `(0,2)` had no pins (sharded review F1/F2) — added, re-run green.
5. **Edge cases:** empty file (size gate), directory (A4), 1 MB header (A3), exact boundaries
   (A1/A2/A6; probe `e03_exact`/`e03_short`; derivation rows 7/18), max/overflow (A6/A7;
   30-digit suite case), retries (idempotent tmp/ cleanup), rollback (pure git; no data).
6. **Security:** all 10 fresh attacker inputs rejected cleanly under ASan; no new file/network/
   shell surface; no secrets; allocation bounded 1:1 by file size (no DoS amplification).
7. **Call stack:** see traces above.
8. **Claims vs evidence:** suite 64/64 (`.work/evidence/final_suite_run.log`); probe
   `PASS E03`/`PASS E04` exit 0 (`.work/evidence/probe_green_final.log`, re-run post-fix);
   extra attacks (`.work/evidence/probe_adv_attacks.log`); grep count 12 (re-run); diff audit
   (re-run); compiler `g++ (GCC) 16.2.1 20260810`.

## Disproven claims

None.

## Unsupported claims

Two were unsupported **during** the session and are now closed:
1. Spec R-V1 scenario "bad magic ≥ 12B" had no test → F1 fixed (suite case 1).
2. Spec R-V7 scenario "zero dimension" had no test → F2 fixed (suite case 3, 6th variant).

Residual documented (not contract violations): the `bad_alloc` path promises false, not
state-unchanged (R-V9 wording is deliberate); `catch(…)` masks internal bugs as spurious
`false` (D5/F4, by design at the I/O shell); a duplicated `'shape': (` token uses the first
occurrence (R-V6); `std::ifstream(nullptr)` exposure is pre-existing and unchanged (same open
call as pre-fix), outside the contract's input list.

## Strongest counterexample

The closest call found **pre-fix** (not in the delivered code): in the assert-enabled suite
build, the pre-fix `better_assert(ifs, …)` turned "missing file" into a process `SIGABRT` —
i.e., the pre-fix code violated the "unopenable path → clean `false`" invariant in debug mode.
This is exactly what D12 removes; the delivered code returns `false` in both build modes
(suite case 1 runs in the assert-enabled build and passes). In the delivered code the strongest
residual counterexample candidate is A7-class inputs, which the overflow-checked multiply
rejects — verified, not assumed.

## Verdict

**PASS** — done condition holds (make test green; ASan release probe exits 0 on E03 + E04;
full suite green; `grep -c 'return false'` on `matrix.hpp` 2499–2590 = 12 > 6), the blast
radius is confined to the allowed set, and no unhandled rejection path or state-inconsistency
was found across 28 crafted inputs (5 suite cases / 18 probe cases / 10 extra attacks, plus the
boundary pair and the bug-restoration red).
