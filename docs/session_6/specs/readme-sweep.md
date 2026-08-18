# Spec: readme-sweep (ReadMe delta landing)

Scope: `ReadMe.md` lands every documented delta from sessions 2–5 plus this
session's FFT and alias content. This session is the sole editor of the
ReadMe deltas (per the S4/S5 handoff note). Session-1 deltas already landed;
this sweep is S2–S6 only.

## Scenarios

#### Scenario: the S2 load_npy delta lands verbatim
- Given the S2 handoff's `load_npy` text (the sole-editor delta)
- When the ReadMe sweep runs
- Then the text is inserted after the `load`/`save` code block (~line 1063),
  quoted verbatim from the S2 handoff (not paraphrased)
- And it states the `.npy` format limitation the S2 handoff records

#### Scenario: the S4 det note lands
- Given the S4 handoff's det delta (exact-zero pivots in pivoted-LU
  detection)
- When the sweep runs
- Then the `det` section (~764–774) gains the one-line exact-zero statement

#### Scenario: the S4 SVD tuple-order and R-20 wide-SVD notes land
- Given the S4 handoff's SVD delta (tuple order `(u, w, v)`) and the risk
  register's R-20 (wide matrices `row > col` untested)
- When the sweep runs
- Then the SVD/`pinv` area (~1764 prose, ~2244 API line) states the tuple
  order and the R-20 wide-SVD disclosure (S4's watch item, S6 owns the
  ReadMe disclosure per the risk register)

#### Scenario: the S4 conv same-mode note lands
- Given the S4 handoff's conv delta
- When the sweep runs
- Then the `conv` section states: same-mode with a valid kernel requires
  `rb >= 1 && cb >= 1`; a 1×1 kernel is pure scaling, not a crash

#### Scenario: the S4 rref precondition note lands
- Given the S4 handoff's rref delta
- When the sweep runs
- Then the `rref` section (~786) states: precondition is now
  `row > 0 && col > 0`; square fully reduced; rectangular (`row < col`)
  reduced with free columns; `row > col` (over-determined) behaves as
  pre-relaxation (E19 pin; documented pre-existing UB under NDEBUG per S5)

#### Scenario: the S4 cholesky bool-return note lands
- Given the S4 handoff's cholesky delta
- When the sweep runs
- Then the Cholesky section states: `cholesky_decomposition` returns `bool`
  (`true` = PD factor computed; `false` = non-PD, strict-positivity guard
  `sum <= 0`, matrix left partially written)

#### Scenario: the S4 statistics promotion note lands
- Given the S4 handoff's statistics delta
- When the sweep runs
- Then the statistics section states: integer/float/double inputs return
  `double` (promoted via `astype<double>`); sample (n−1) variance

#### Scenario: the S5 NDEBUG policy lands verbatim
- Given `docs/session_5/design.md` §4 (the C7 policy text)
- When the sweep runs
- Then a new `#### assertions and NDEBUG` section (~build area, 1364)
  reproduces the S5 policy text verbatim: assertions are debug-only,
  `better_assert` is a no-op under NDEBUG, public contracts are precondition
  documents not runtime guarantees

#### Scenario: the new fft section and alias-retirement table land
- Given this session's FFT implementation and A2 retirements
- When the sweep runs
- Then a new `#### fft -- fast Fourier transform` section (~after `lu`,
  1800) documents: the radix-2 vs naive-fallback rule, complexity, the NumPy
  convention + `ifft` normalization, the `fftshift`/`ifftshift` convention +
  fused-design deviation note, and the alias-retirement table
  (`random→rand`, `random_like→rand_like`, `pinverse→pinv`,
  `svd_inverse→pinv`, `feng::det(m)→m.det()`)
- And the rand example (~1277) reads `rand(1,2,3.0)`

#### Scenario: the sweep is complete and verified
- Given the finished ReadMe
- When each scenario above is checked against the file
- Then every delta is present, and the checklist in `design.md` §4 is marked
  done (the handoff records the line-by-line verification)
