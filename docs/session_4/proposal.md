# Session 4 — Proposal (capability breakdown)

Source of authority: `docs/session_4_contract.yaml` (dominant for how-to) + `docs/prd.md`
line 11 (dominant for what-to). This proposal narrows the contract into four capability units,
each self-contained and independently testable. No scope additions.

## Capabilities

### 1. `stat-promotion` — C8: statistics return `double` for real value types
- **What:** `mean`, `variance`, `standard_deviation` return `double` for integer and floating
  point matrices (complex path preserved). Integer matrices are promoted via `astype<double>()`
  before the unchanged formula; `double` matrices take a copy-free fast path.
- **Why:** `sum(int)/size_t` is unsigned integer division (`unsigned long`, truncated —
  negative sums wrap); `m - mean(m)` on integer matrices does not compile; variance/stddev on
  `int` are hard compile errors pre-fix (probe p0/p0b/p0c).
- **In:** the three function bodies at `matrix.hpp:7769/7775/7781` (type-class branches only);
  `tests/cases/mean.hpp` (add int/float cases + type static_asserts); E10.
- **Out:** `sum`'s int accumulator (pre-existing, no in-repo int caller — R-15 watch),
  population-vs-sample formula (kept, PRD C-10), complex statistics, `matrix_details::reduce`.
- **Acceptance (contract):** E10 content + types; invariant pins {1×2, 2×2, 1×1};
  `make test` + `make example` (stdout identical).

### 2. `conv-same-kernel` — C9: `conv` "same" mode accepts 1×1 (and non-square) kernels
- **What:** the copy-pasted second `rb > 1` assert becomes a `cb`-checking `>= 1` precondition;
  both directions (rb≥1, cb≥1) pinned.
- **Why:** a well-defined 1×1 "same" convolution (scaling) aborts in debug builds
  (probe p1, SIGABRT at matrix.hpp:6755).
- **In:** the two assert lines at `matrix.hpp:6754-6755`; new `tests/cases/conv_same.hpp`;
  E11.
- **Out:** the full/valid/same slice arithmetic (verified correct by the 2026-07-13 review),
  any other `conv` line.
- **Acceptance (contract):** E11; rb==1,cb>1 and rb>1,cb==1 separately; valid-mode regression
  case; asserts compile under `-Wall -Wextra`.

### 3. `rref-domain` — C10: `rref`/`gauss_jordan_elimination` accept square (and any non-empty) systems
- **What:** the precondition relaxes from `row < col` to `row > 0 && col > 0`.
- **Why:** square systems are the most common use case and are rejected by the assert
  (probe p2, SIGABRT at matrix.hpp:6486).
- **In:** the single assert at `matrix.hpp:6486`; new `tests/cases/rref.hpp`; E12.
- **Out:** the algorithm body (the pre-existing `row > col` OOB is documented + ASan-pinned,
  not repaired — the contract's own out-of-scope clause); the `std::optional` return type;
  the 1e-10 singular exit (kept).
- **Acceptance (contract):** E12; square singular → nullopt, no hang; wide regression;
  row>col behaves identically to pre-relaxation (ASan before/after pair).

### 4. `cholesky-guard` — P2 (cholesky): `cholesky_decomposition` reports failure
- **What:** signature `void` → `bool`; a strict positive-definiteness guard at the diagonal
  step (`sum <= 0 → false` before the `sqrt`); returns `true` on completed factorization.
- **Why:** non-PD input silently produces NaN entries with no failure channel (probe p0:
  `a[1][1] = -nan`); a void return cannot report failure (P2, PRD line 11).
- **In:** the diagonal-step guard + return type at `matrix.hpp:5766-5784`; new
  `tests/cases/cholesky.hpp`; E13.
- **Out:** the factorization arithmetic; the upper-triangular zero-fill; `better_assert`
  square-shape precondition (kept).
- **Acceptance (contract):** E13; PD → true with `a·aᵀ ≈ m`; non-PD → false, no NaN;
  PSD-singular → false; `1×1 {0}` → false; `1×1 {4}` → true.

## Risk table (carried from the contract; all medium, plan-of-record = TDD + targeted check)

| Risk | Trigger | Severity | Evidence to collect |
|---|---|---|---|
| E10 sample-variance trap | asserting population `0.5` instead of `√0.5` | medium | exact `√0.5`/`√(1/3)` pins (probe `-O1` + suite tolerance) |
| E11 "1×1 kernel" misread | re-asserting `rb > 1` or slicing with `rb-1` underflow | medium | rb==1, cb>1 and rb>1, cb==1 probes + valid regression |
| E12 singular square hang | division-by-zero path in gauss_jordan | medium | singular-square → `nullopt` within 1s, no abort |
| E13 false-rejection of PD | guard too broad (e.g. `sum < tiny_eps → false`) | medium | PD `[[4,2],[2,3]]` → `true`, `a·aᵀ ≈ m` |

## Files that will change (blast radius check)

`matrix.hpp` (4 regions, ~15 lines total), `tests/test.cc` (3 include lines),
`tests/cases/mean.hpp` (additions), `tests/cases/conv_same.hpp` (new), `tests/cases/rref.hpp`
(new), `tests/cases/cholesky.hpp` (new), `docs/eval_seed_cases.md` (E10–E13 → promoted),
`docs/risk_register.md` (closeout watch items), `docs/session_4/**` (phase docs),
`.work/probes/*`, `.work/evidence/*`, `.work/handoff_session_4.md`.
All within the contract's `allowed_files`. Nothing else.
