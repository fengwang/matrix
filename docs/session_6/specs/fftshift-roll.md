# Spec: fftshift-roll (C13)

Scope: `fftshift`/`ifftshift` in `matrix.hpp`. The fused transform+shift design
is kept (documented NumPy deviation). The swap-of-halves remap is replaced by
a circular roll of `(n+1)/2` per axis, applied to both functions. No public
signature changes.

## Scenarios

#### Scenario: even dimensions keep the historical remap (bit-identity)
- Given even `row` and `col` (e.g. 4×8)
- When `fftshift` is applied
- Then the remap is the swap-of-halves permutation (roll by `n/2`),
  bit-identical to the pre-fix behavior (pre-fix probe: n=4 both give
  `(2,3,0,1)`)
- And the even-dim regression pin holds in the suite and probe

#### Scenario: odd row dimension matches the NumPy convention
- Given a 3×1 column `[1, 2, 3]`
- When `fftshift` (or `ifftshift`) is applied to its spectrum
- Then the row order is `(1, 2, 0)` for both functions (E17)
- And for a 5×1 input the row order is `(2, 3, 4, 0, 1)` for both functions
  (C13 note; pre-fix was `(3, 4, 2, 0, 1)` — the documented bug)

#### Scenario: odd column dimension matches the NumPy convention
- Given a 1×3 row `[1, 2, 3]`
- When `fftshift` (or `ifftshift`) is applied to its spectrum
- Then the column order is `(1, 2, 0)` for both functions (E17)

#### Scenario: the shift is a pure reindex of the transform result
- Given the transform result `X` of an input `x`
- When `fftshift(x)` is computed
- Then `fftshift(x) == shift_roll(fft(x))` where `shift_roll` maps
  `new[i] = old[(i − s) mod n]` with `s = (n+1)/2` per axis
- And the fused design (transform then shift) is preserved and documented
  as a deliberate NumPy deviation in the ReadMe and handoff

#### Scenario: shift and transform compose on non-power-of-two shapes
- Given a 3×5 or 5×3 matrix (odd dims, fallback transform path)
- When `fftshift`/`ifftshift` is applied
- Then the result equals `shift_roll(naive_dft(x))` within tolerance
  (the roll applies to whatever transform path ran)
