# Spec: fft-tests (E16/E17/E18 + suite coverage)

Scope: the test layer for P1/C13/A2. New `tests/cases/fft.hpp`, the verbatim
E16/E17 probe, the E18 compile probe, and the F2 canonicalization of
`tests/cases/pinv.hpp`. Suite policy R-19 is respected: suite assertions use
tolerances on finite values only; exact pins live in the `-O1` probe.

## Scenarios

#### Scenario: the E16/E17 probe passes post-fix (contract-verbatim)
- Given `.work/probes/E16_E17.cc` exactly as written in the contract
- When it is built with `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s6 .work/probes/E16_E17.cc && .work/probe_s6`
- Then (post-fix) it exits 0 and prints `E16_E17 PASS`
- And (pre-fix, recorded as baseline) it fails or reports the degenerate
  values, with the failure logged in `.work/evidence/s6_e16_e17.log`
- And the probe asserts: `fft` of the 8×8 delta at (0,0) has all elements
  within 1e-9 of 1.0; `‖ifft(fft(x)) − x‖∞ < 1e-9` for the fixed 8×8 input
  `x[r][c] = 1.0 + 0.5·cos(r)·sin(c)`

#### Scenario: the fft suite case covers fast and fallback paths
- Given `tests/cases/fft.hpp` (registered in `tests/test.cc` at the
  alphabetical position between `fabs` and `flip`)
- When `make test` runs
- Then an 8×8 differential case (fast path) and a 6×8 differential case
  (fallback path) both match the embedded corrected-naive oracle within
  tolerance (1e-9 double; 1e-4 float)
- And a 126×128 case exercises the fallback at the contract's adversarial
  size
- And the embedded oracle is a self-contained copy of the corrected naive
  DFT, frozen after S6 (R-18), with a header comment recording the F1
  provenance (pre-fix loop had the `x[r][c]` data-index bug)

#### Scenario: the E17 shift pins hold
- Given the suite's shift scenarios
- When `fftshift`/`ifftshift` are applied to 1×3 and 3×1 inputs
- Then both functions produce the order `(1, 2, 0)` (E17)
- And for 4×1/1×4 inputs both produce `(2, 3, 0, 1)` (E17 even-n pin)
- And the C13 odd-5 case produces `(2, 3, 4, 0, 1)` (contract note; pre-fix
  was `(3, 4, 2, 0, 1)`)

#### Scenario: even-dimension fftshift is bit-identical to pre-fix
- Given a 4×8 input
- When `fftshift` is applied post-fix
- Then the result matches the pre-fix swap-of-halves remap of the transform
  result (the suite compares against a hand-rolled swap-of-halves copy of the
  old remap; the probe asserts the same against the corrected fast transform)
- And this pin protects the contract's even-n regression invariant

#### Scenario: normalization is pinned exactly once
- Given a finite 8×8 input `x`
- When `ifft(ifft(x))` is computed
- Then `‖ifft(ifft(x)) − x/(R·C)²‖∞ < 1e-9` (the factor appears once per
  `ifft` call, never on `fft`; a missing or doubled factor fails this)

#### Scenario: edges and strides
- Given 1×8, 8×1, 1×1, and 0×0 inputs
- When `fft`/`ifft` are applied
- Then 1×8/8×1 match the oracle (row-only / column-only fast path, stride-1
  and stride-8 cases), 1×1 is the identity for `fft` and `1·ifft` for `ifft`,
  and 0×0 returns empty (pre-fix guard)

#### Scenario: the E18 compile probe passes (contract-verbatim)
- Given the A2 deletions
- When `.work/probes/E18_a2.cc` (using `feng::random`, `feng::random_like`,
  `feng::pinverse`, `feng::svd_inverse`, free `feng::det`) is compiled
- Then it fails to compile, with one of the retired identifiers named in the
  error
- And a companion probe using `feng::pinv` + `feng::rand` compiles and runs
- And both outcomes are logged in `.work/evidence/s6_e18.log`

#### Scenario: pinv.hpp is canonicalized (F2, reported refinement)
- Given `tests/cases/pinv.hpp`
- When the A2 change lands
- Then the "pinv == pinverse" scenario is removed, the TEST_CASE is renamed
  to "Matrix pinv", the header comments reference `matrix_details::pinv_core`,
  and the remaining scenarios pass unchanged
- And no other line of the file is modified (the refinement is minimal and
  logged in `failure_arbiter.md` F2 + the handoff decision log)

#### Scenario: eval seeds are promoted
- Given the closeout
- When `docs/eval_seed_cases.md` is updated
- Then E16 and E17 are added as live entries (the contract's probe bodies,
  with build commands and the S6 closeout note) and E18 is marked live per
  the evidence map's A3 row
- And `docs/evidence_map.md` gains the E16/E17 rows and the C-08 correction
  (pre-fix loops were not a DFT; F1)
