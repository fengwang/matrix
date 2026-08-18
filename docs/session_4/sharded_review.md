# Session 4 Sharded Review (in-process, fresh-context simulation)

**Mode note:** subagents do not work on this host (S1 record: the single configured model
exhausts the 16K output budget; re-confirmed S4). This review was run **in-process as a
fresh-context simulation** per `docs/prompts/sharded_review.md`: 4 shards, each re-derived
from the shard's own diff + the contract/spec inputs only (no memory of authoring), 6 axes
per shard (correctness, readability/simplicity, security/safety, tests, architecture,
performance). Risk level: **medium** (per session contract).

**Review base:** `git diff c3067cf..HEAD` (S4 pre-flight commit → task 4 commit 53a77fc):
`matrix.hpp` (+64/−10), `tests/cases/{mean,conv_same,rref,cholesky}.hpp`, `tests/test.cc`
(+3 includes), evidence log.

**Shards**
- **A** — `matrix.hpp` C8 statistics (`mean`/`variance`/`standard_deviation`, ~7775–7847)
- **B** — `matrix.hpp` C9 conv same-mode asserts (6766–6769) + C10 gauss_jordan precondition (6499)
- **C** — `matrix.hpp` P2 `cholesky_decomposition` (5766–5797)
- **D** — all four test files + `tests/test.cc` includes + seed/risk-register doc deltas

---

## Shard A — C8 statistics

**Verdict: PASS (1 Low finding).**

- *Correctness:* the three-way dispatcher matches the spec `stat_promotion.md` exactly:
  complex → pre-fix expression verbatim (verified line-by-line against the pre-flight
  baseline; `mean(complex)` stays complex-typed, `variance/stddev(complex)` stay
  ill-formed — pre-existing, probe p0d/p0d2); `double` → copy-free fast path identical to
  the pre-fix expression; other real types → `astype<double>()` **before** the unchanged
  formula. All real return paths deduce `double` (static_asserted in the test; probe E10
  exact). The promotion-before-formula ordering is load-bearing: `operator-(matrix<T>,
  const T&)` requires the scalar to be exactly `T`, so a double mean applied to an int
  matrix would truncate (probe p0: pre-fix `mean(matrix<int>)` = `unsigned long`, value 1
  for {1,2}; unsigned integer division wraps for negative sums). The `size<=1` branch
  returns `double{}` for real types and `value_type{}` for complex — required for
  consistent `auto` deduction (int{} vs double sqrt would be ill-formed).
  Empty-matrix edge: `size==0` is unreachable for constructible matrices (S1 watch item:
  0-size constructibility is conditional; constructor policy unverified). Pre-fix int mean
  on 0-size would be unsigned division-by-zero (UB); post-fix it is `0.0/0 = NaN` —
  strictly better, no new exposure. No off-by-one: `d.size() - 1` is guarded by the
  `size <= 1` early return.
- *Readability:* the dispatcher shape (complex / double / promoted) is the spec-mandated
  one; the comment in `mean` states **why** promotion happens (unsigned division + the
  `operator-` truncation), which the other two inherit. The double fast path is not dead
  weight: it is the pre-fix expression verbatim and avoids an O(n) allocation for the
  most common value type (performance axis). No clever tricks; the ternary-free `if/else`
  is straightforward.
- *Security:* none (pure numeric; no I/O, no untrusted input).
- *Tests:* content pins with exact values + type `static_assert`s; the existing
  random-double case (sizes 1..9) is untouched and passes — regression net for the double
  path. **Gap (L1):** no negative-integer-matrix case. The unsigned-wrap hazard (negative
  sum → wrap in the pre-fix `unsigned` mean) is the specific regression the fix exists to
  remove, and nothing in the suite would catch a reversion to unsigned arithmetic on
  negative input.
- *Architecture:* reuses the canonical in-library helpers (`astype`, `reduce`-based
  `sum`, `pow`); no new abstraction; the type-class dispatch is the one place the type
  boundary becomes explicit, per spec.
- *Performance:* int/float statistics now allocate one `double` copy (O(n), documented in
  design §1 and the risk register); double matrices unchanged (copy-free); no hot-path
  regressions in the suite build.

**Finding L1 — Low — `tests/cases/mean.hpp` (shard A, tests axis).**
- Evidence: no test feeds a negative integer matrix to `mean`/`variance`/`standard_deviation`; the pre-fix failure mode was *unsigned* integer division (negative sum wraps to a large positive) — a positive-only pin cannot distinguish signed-from vs unsigned-from arithmetic after promotion.
- Violated invariant: spec `stat_promotion.md` scenario "integer matrices promoted before division" — the wrap case is the discriminating input.
- Impact: a regression to pre-fix integer arithmetic would pass the current suite.
- Smallest safe fix: add a negative int case (e.g. 1×2 {−1, 2}: mean 0.5, variance 0.25, std √0.5) to the C8 test case.
- Confidence: high.

## Shard B — C9 + C10 asserts

**Verdict: PASS (1 Low finding).**

- *Correctness:* C9 — `rb >= 1` / `cb >= 1`: the second assert previously re-tested `rb > 1` (copy-paste; probe p1 SIGABRT at 6755 on a 1×1 kernel). With `rb >= 1`, the slice arithmetic `(rb-1)>>1` cannot underflow (minimum `(1-1)>>1 = 0`), and a 1×1 kernel's same-mode slice is the whole full conv = scaling (probe E11 exact). Even-kernel semantics (`(rb-1)>>1` floor) are **unchanged** — only the rejected domain changed; the fix also repaired a message/condition mismatch (the message said "at least 1" while the condition demanded "greater than 1"). C10 — `row > 0 && col > 0` matches the spec `rref_domain.md` domain exactly; the 1e-10 singular exit is untouched (probe: singular → nullopt before any division); the `row > col` OOB is pre-existing and out of scope (ASan pair p3 identical in substance pre/post; documented in the test header + risk register). Both asserts keep the house `cond && "msg"` style (C10) / `cond, "msg ", arg` style (C9) matching their pre-fix lines.
- *Readability:* two single-line condition changes; messages unchanged (C9) / typos corrected in the rewritten text (C10, S3 D7 precedent).
- *Security:* none.
- *Tests:* E11 content (scaling), both narrow directions (rb1cb2, rb2cb1), and a valid-mode
  regression pin (2×3 kernel, slice arithmetic). **Gap (L2):** no same-mode pin for
  `rb > 1 && cb > 1` — the mainstream pre-fix usage (e.g. 2×2 kernel on a 3×3). The fix
  touched the assert guarding exactly that mainstream path; nothing pins that its
  *content* is unchanged, only that the narrow kernels now work and that *valid* mode is
  unchanged.
- *Architecture:* minimal, sanctioned lines only; no flow changes.
- *Performance:* none.

**Finding L2 — Low — `tests/cases/conv_same.hpp` (shard B, tests axis).**
- Evidence: the four scenarios cover the newly-accepted domain (1×1, rb1cb2, rb2cb1) and the valid branch, but no scenario covers a same-mode call with a kernel ≥ 2×2, i.e. the behavior that existed pre-fix and must be content-identical post-fix.
- Violated invariant: spec `conv_same_kernel.md` "out of scope: … full/valid slice arithmetic … unchanged" — unchanged is a claim that needs a pin.
- Impact: a slice-offset or full-conv arithmetic regression in the ≥ 2×2 same path would pass the suite.
- Smallest safe fix: add a 3×3 matrix × 2×2 sum kernel, "same", with a hand-derived full-conv trace pin.
- Confidence: high.

## Shard C — cholesky guard

**Verdict: PASS (1 Low finding).**

- *Correctness:* the guard sits inside `i == j`, **before** `std::sqrt` (spec
  `cholesky_guard.md`), on `sum <= value_type(0)` — strict positivity, boundary `<=` per
  C-11 (the contract's adversarial cases 1×1 {0} and PSD-singular dominate the `in_scope`
  line's `sum < 0` wording; recorded in the seed file and risk register). Returning
  `false` before the sqrt leaves `a` in a defined state (test pins the exact preserved
  values: a[0][0]=1, a[1][0]=2, a[1][1]=input 1). The off-diagonal divide `sum / a[i][i]`
  is safe without a separate guard: after the guard every diagonal step is
  `sqrt(sum > 0) > 0`. Complex path excluded via `if constexpr (!ComplexMatrix<Matrix1>)`
  (no ordering for complex; legacy preserved; zero in-repo complex callers — risk
  register). The `void→bool` change has zero in-repo callers (evidence map §1) and
  matches PRD §5 row 11. The `better_assert(m.row() == m.col())` and the zero-fill loop
  are untouched. n==1 traced by hand: guard on `m[0][0]`, sqrt, no fill iterations.
- *Readability:* the ternary became an explicit if/else — clearer, same expression count;
  the comment states why (silent NaN) and the complex carve-out.
- *Security:* none (numeric; the non-square assert is pre-existing debug-only — not an I/O
  boundary, R-06 N/A).
- *Tests:* five adversarial inputs (non-PD, PD + factor + a·aᵀ≈m, PSD-singular, 1×1 {0},
  1×1 {4}). One mid-task TEST_DEFECT was caught and fixed (the a·aᵀ (0,0) element summed a
  column instead of a row — 5 instead of 4; the library values were correct). **Gap
  (L3):** all PD coverage is 2×2 — a 3×3 PD case would pin the multi-step `inner_product`
  accumulation across several diagonal steps, and a float value_type case would pin the
  `value_type(0)` guard under the non-double type.
- *Architecture:* the guard is local to the one function whose contract changed; no shared
  helper introduced (correct — this is the only PD check in the library).
- *Performance:* the guard adds one comparison per diagonal step (n²/2 → n); negligible;
  early-exit on non-PD input is a *gain* (pre-fix continued into NaN arithmetic).

**Finding L3 — Low — `tests/cases/cholesky.hpp` (shard C, tests axis).**
- Evidence: only 2×2 (and 1×1) inputs exercised; no 3×3 PD pin and no float value_type pin.
- Violated invariant: spec `cholesky_guard.md` "PD matrices return true with the unchanged factor" — the multi-step factor path is under-pinned.
- Impact: an `inner_product` accumulation or indexing regression at depth ≥ 3 (or a float-precision guard misfire) would pass the suite.
- Smallest safe fix: add a 3×3 SPD with an exact hand-derived factor (m = L·Lᵀ with L = [[2,0,0],[1,2,0],[0.5,0.5,1]] → m = [[4,2,1],[2,5,1.5],[1,1.5,1.5]]) and a 1×1 `matrix<float>{4.0f}` → true, `a[0][0] == 2.0f`.
- Confidence: high (values hand-derived; verified against the suite).

## Shard D — tests + doc deltas

**Verdict: PASS (no findings of its own; consumes L1–L3).**

- *Correctness:* the three `tests/test.cc` includes land at the alphabetical slots
  (cholesky after ceil L17, conv_same after cos L19, rref after rint L60) per the plan;
  the suite grew 69 → 73 cases with every new case content-asserting. Tolerances (1e-10
  .. 1e-12) and finite-only values make every suite assertion R-19 safe under the
  `-Ofast` suite build; exact `==` pins live only in the `-O1` probe (E10_E13), which
  prints PASS (finite exact values: powers of two, same-operand `std::sqrt` comparison).
  The `rref.hpp` header documents the row>col exclusion with the ASan probe reference;
  `conv_same.hpp` documents the pre-fix abort site; `cholesky.hpp` documents the guard
  semantics. The E10_E13 probe excludes row>col by design (pre-existing UB) and documents
  why.
- *Readability:* test files follow the established case-file pattern (named scenario
  blocks, `want` arrays, looped REQUIREs, spec/eval-seed references in comments).
- *Security:* none.
- *Tests:* tautology check — every assertion compares against an independently-derived
  value (hand derivations in design.md, NumPy cross-checked; the existing random-double
  mean case remains as an independent cross-check). No assertion merely checks
  "it ran".
- *Architecture:* the deterministic probe is a single TU per the contract's
  `deterministic_check` line (verbatim build command, verbatim PASS output — evidence
  logged); seeds E10–E13 promoted with probe + permanent-home references, matching the
  S1–S3 promotion format.
- *Performance:* the new suite cases are tiny fixed-size inputs (< 40 elements each);
  suite runtime impact negligible (build+run time tracked in the closeout evidence).

---

## Consolidated findings

| ID | Severity | Shard | Location | Summary | Fix |
|---|---|---|---|---|---|
| L1 | Low | A/tests | `tests/cases/mean.hpp` | no negative-int stats pin (unsigned-wrap regression undetectable) | add 1×2 {−1, 2} case |
| L2 | Low | B/tests | `tests/cases/conv_same.hpp` | no same-mode pin for kernel ≥ 2×2 (mainstream pre-fix usage unpinned) | add 3×3 × 2×2 sum kernel, hand-derived trace |
| L3 | Low | C/tests | `tests/cases/cholesky.hpp` | no 3×3 PD pin, no float value_type pin | add both (exact factors) |
| L4 | Low | B/tests | `tests/cases/rref.hpp` | square case never exercises the row-swap path (diagonal pivot only) | add 2×2 {{0,1},{1,0}} → I (swap exercised) |

**No Critical or High findings.** All four Lows are test-coverage gaps in already-correct
library code (the library-side changes verified correct on the correctness axis of every
shard); per S3 convention they are fixed in-session before closeout and the suite +
deterministic probe are re-run.

**Resolution (closeout):** L1–L4 all fixed in `tests/cases/{mean,conv_same,rref,cholesky}.hpp`
and re-verified (suite 73 cases all pass; E10_E13 probe PASS). Two TEST_DEFECTs were caught
**while writing the fixes** (both mine, both test-side, library correct):

1. **L1 fix arithmetic error:** `standard_deviation({−1,2})` is `sqrt(sum/(n−1)) =
   sqrt(4.5/1) = √4.5 ≈ 2.12132`, not `sqrt(2.25) = 1.5` (the n−1 formula divides the
   *sum of squared deviations*, 4.5, not the variance, 2.25). First run failed with the
   measured 2.12132, confirming the library's value; the pin now uses `std::sqrt(4.5)`.
2. **L2 fix hand-derivation model error:** the first 2D-kernel pin exposed that the
   library's conv correlates with the kernel's **bottom-right element anchored**
   (f(r,c) = Σ A[r−rb+1+i][c−cb+1+j]·K[i][j]), not the top-left anchor used in my
   hand-derived full-conv traces. For 1D kernels the two conventions coincide, which is
   why scenarios (a)–(d) (1×1, 1×2, 2×1 kernels, valid 2×3) were consistent with both
   models. The measured full 4×4 (`{{1,3,5,3},{5,12,16,9},{11,24,28,15},{7,15,17,9}}`)
   is the pre-fix behavior (the fix touched only the assert conditions), and the pin now
   records the measured values with the convention documented in the comment.
   **Record value:** any future work on `conv` slice/offset arithmetic should use
   bottom-right-anchored correlation traces (or just measure) — the 1D-trace shortcut is
   not valid for 2D kernels.
3. **L4 label cleanup + L3 3×3 SPD/float pins:** values hand-derived (L = [[2,0,0],
   [1,2,0],[0.5,0.5,1]], m = L·Lᵀ, all binary-exact; algorithm trace verified step by
   step) — passed on first run.

**Checks not run in this review:** no independent second-model review (host constraint,
mode note above); the review is a simulation of the shard prompts, so "independence" is
input-discipline (diff + contract only), not process independence.
