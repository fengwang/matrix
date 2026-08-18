# Spec — `stat-promotion` (C8): statistics return `double` for real value types

## Requirement
`feng::mean`, `feng::variance`, `feng::standard_deviation` (matrix.hpp:7769/7775/7781) return
`double` for integer and floating-point value types, computed on a double-promoted copy for
non-double real types, with the formulas unchanged.

## Constraints
- Formula preservation: `mean = Σ/n`; `variance = mean((m−μ)²)` (population);
  `standard_deviation = sqrt(Σ(m−μ)²/(n−1))` — **`n−1` kept** (PRD revision C-10; E10 pin
  `√0.5 ≈ 0.70711`, not the population `0.5`).
- `double` matrices: copy-free (no `astype` no-op copy); values unchanged bit-for-bit
  (same expression).
- `float` matrices: values unchanged within rounding (promotion to double of the same
  formula).
- `complex` value type: legacy expression verbatim (complex `mean` stays `complex`-typed;
  complex `variance`/`stddev` were ill-formed pre-fix — same `operator-` root cause — and
  remain so; no in-repo complex callers).
- `size ≤ 1` stddev branch: `double{}` for real types (was `value_type{}`), legacy
  `value_type{}` for complex (auto-deduction consistency).
- No new epsilon, no new dependency, no change to `sum`/`reduce`/`operator-`/`astype`.
- `constexpr` specifier and `requires Matrix< Mat >` constraint unchanged.

## Acceptance (from contract)
1. E10: `matrix<int>{1,2,{1,2}}` → mean `1.5`, variance `0.25`,
   `standard_deviation ≈ 0.70711` (`√0.5`); each is `double` (type check).
2. Same for `matrix<float>` and `matrix<double>` inputs (float/double matrices unchanged
   within rounding).
3. Invariant pins (content-asserting, not just compiles):
   - int 1×2 `{1,2}`: 1.5 / 0.25 / `√0.5` (exact-binary).
   - int 2×2 `{1,2;1,2}`: 1.5 / 0.25 / `√(1/3)` (shape-ambiguity killer).
   - int 1×1 `{7}`: mean 7.0, variance 0.0, stddev 0.0 (`size≤1` branch).
4. `make test` (new `tests/cases/mean.hpp` additions + existing double case) passes.
5. `make example` stdout identical to baseline.
6. Deterministic probe E10 part prints exact values + types (`-O1`, no fast-math).

## Out of scope
`sum`'s integer accumulator (pre-existing, R-15 watch), population-vs-sample decision,
complex statistics, `matrix_details::reduce`, any `noexcept`/`constexpr` semantics beyond
what the unchanged formula requires.
