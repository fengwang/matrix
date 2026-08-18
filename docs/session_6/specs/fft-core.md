# Spec: fft-core (P1)

Scope: `fft`/`ifft` in `matrix.hpp` become the correct 2-D DFT with a fast
power-of-two path and the retained (corrected) naive fallback; `ifft` carries
the single `1/(R·C)` normalization. No public signature changes. No new
dependencies. See `failure_arbiter.md` F1 for the pre-fix finding (the pre-fix
loops are not a DFT; the oracle is the *corrected* naive DFT).

## Scenarios

#### Scenario: fast path selected when both dimensions are powers of two
- Given a matrix with `row` and `col` both powers of two (e.g. 8×8, 1×8, 8×1, 1×1)
- When `fft(x)` or `ifft(x)` is called
- Then the separable radix-2 path runs (O(R·C·log(R·C)))
- And the result equals the reference DFT within tolerance (1e-9 double)

#### Scenario: fallback selected when any dimension is not a power of two
- Given a 6×8, 126×128, or 5×4 matrix
- When `fft(x)` or `ifft(x)` is called
- Then the whole-matrix naive DFT (corrected loop, O(n⁴)) computes the result
- And the result equals the embedded oracle within tolerance (differential test)

#### Scenario: fft is unnormalized (NumPy fft2 convention)
- Given the 8×8 delta matrix with 1.0 at (0,0) and 0 elsewhere
- When `fft` is called
- Then every element of the result is 1.0 within 1e-9 (E16)

#### Scenario: ifft normalization applied exactly once, on ifft only
- Given any finite 8×8 input `x`
- When `y = ifft(fft(x))`
- Then `‖y − x‖∞ < 1e-9` (identity round-trip; E16)
- And `‖ifft(ifft(x)) − x/(R·C)²‖∞ < 1e-9` (the factor appears once per call,
  never on `fft`)

#### Scenario: naive fallback is the corrected reference
- Given the retained naive DFT loop
- When it is moved to `fft_private::naive_dft`
- Then its data index is `x[r_][c_]` (input index; the pre-fix `x[r][c]`
  bug is the one-token fix)
- And its kernel, `add_complex` promotion, and structure are otherwise
  unchanged (frozen after S6 per R-18 as the oracle's provenance)

#### Scenario: empty input
- Given a 0×0 matrix
- When `fft` is called
- Then an empty result is returned (pre-fix guard behavior preserved)

#### Scenario: input is not mutated
- Given a matrix `x` and a copy `x0`
- When `fft(x)`, `ifft(x)`, `fftshift(x)`, or `ifftshift(x)` is called
- Then `x == x0` afterward (pure Calculation; local buffer only)

#### Scenario: benchmark target
- Given the serial benchmark (100 random 512×512 transforms, serial vs serial,
  `-O1` or better, single run, logged to `.work/`)
- When the radix-2 path runs
- Then it is 10–100× faster than the pre-fix O(n⁴) loop on 512×512
  (PRD goal 3; the old code's benchmark is recorded pre-fix in the evidence
  log so the ratio is measured, not asserted)
