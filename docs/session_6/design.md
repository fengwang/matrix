# Session 6 — Design

Status: complete. Implements the decisions in `proposal.md`/`interview.md`/
`brainstorming.md`. All matrix.hpp anchors were re-verified against HEAD d5e7b56 at
session start (contract line numbers are stale; names are authoritative).

## 1. Code layout in `matrix.hpp`

The FFT region today (line anchors verified this session):

| Symbol | Line (HEAD) |
|---|---|
| `namespace fft_private { add_complex; make_omege (local lambda in fft); naive loops }` | 6409–6457 |
| `fft` (public template) | 6459–6457 (ends 6457) / `fftshift` 6460–6480 |
| `namespace ifft_private` (empty) | ~6544 |
| `ifft` (public, contains its own make_omege + naive loops) | 6556–6590 / `ifftshift` 6591–6611 |
| free `det(m)` (MATLAB alias) | 4410–4414 |
| `svd_inverse` / `pinverse` / `pinv` | 5307–5314 / 5317–5320 / 5321–5325 |
| `rand` (S5, 3 forms) | 5329–5352 |
| `random` (2 overloads) / `rand_like` / `random_like` / `randn_like` | 5353–5358 / 5363–5367 / 5369–5372 / 5374–5379 |
| first `matrix_details` block | 139–1190 |
| second `matrix_details` block (`map`/`reduce` etc.) | 4030–4156 (closes at 4156) |
| `singular_value_decomposition` (out-param + tuple) / `svd` | ~5000–5303 (feng namespace, after both blocks) |

Changes:

1. **`fft_private`** becomes the real private core (the empty `ifft_private` is
   deleted; its only claimant is this session):
   - `is_power_of_two( std::uint_least64_t n )` → `n != 0 && (n & (n-1)) == 0`.
   - `twiddle_table< T >( n, bool inverse )` → `std::vector< std::complex< T > >`
     with `w[k] = exp(∓2πi·k/n)` for `k = 0 … n/2−1` (n ≥ 2, a power of two).
     Uses `std::polar( 1.0, ∓2π·k/n )` on `std::complex<double>` then converts to
     `std::complex< T >` (float path keeps full-precision table construction,
     matching the oracle's `double`-theta `make_omege`).
   - `radix2_fft_1d< T >( buf, start, stride, n, w const&)` → in-place DIT:
     bit-reversal pass (`rev = bitreverse_m(i); if rev > i swap`), then
     `for ( len = 2; len <= n; len *= 2 ) { half = len/2; for each block base
     b in {0, len, …, n−len} } v[s+k] = u + w[k·(n/len)]·t; v[s+k+half] = u − t`
     with `u = v[start+b+k·stride]`, `t = v[start+b+(k+half)·stride]`.
   - `naive_dft< T >( Mat const& x, bool inverse )` → the existing 4-loop naive
     structure, **data index corrected to `x[r_][c_]`** (F1), twiddle
     `make_omege` kept (moved to `fft_private` as a proper function taking
     `auto k, auto n, auto N`, computing `exp(−2πi·k·n/N)`); the inverse selects
     `std::conj` of the forward twiddle. `add_complex` promotion untouched.
     Returns `matrix< typename add_complex< T >::result_type >`.
2. **`fft( Mat const& x )`** (public, `fftshift`'s callee): guard
   `R == 0 || C == 0` → empty (existing). `is_power_of_two(R) &&
   is_power_of_two(C)` → buffer path (rows stride 1 len C, then columns stride C
   len R) else `naive_dft< T >( x, false )`.
3. **`ifft( Mat const& x )`** (public, takes complex `Mat`): same structure with
   `inverse = true`, then a single final scaling of the result by
   `1.0 / ( R * C )` (applied element-wise; exactly once; `fft` never scales).
4. **`fftshift` / `ifftshift`**: keep the shape (transform, then remap) but the
   remap becomes `shift_roll` (new, in `fft_private`): allocates the result
   matrix; for rows, `out[i][c] = in[(i + R − sR) % R][c]` with `sR = (R+1)/2`;
   same for columns with `sC = (C+1)/2`. (Equivalent to the `swap_ranges`
   result for even n; matches the pinned NumPy roll for odd n.)
5. **A2 deletions/moves** (names, anchors verified):
   - `svd_inverse` (5307–5314) and `pinverse` (5317–5320): deleted. The S3 body
     (SVD out-param decomposition, invert `w` where `|w| > 1.0e-10` — a
     **hard-coded strict threshold, no tolerance parameter**; the S3 handoff's
     "tolerance=0.01" phrasing does not match the code) moves verbatim into the
     second `matrix_details` block as
     `matrix_details::pinv_core( matrix< T, A > const& a ) → matrix< T, A >`
     (placed before the block's close at 4156; the unqualified
     `singular_value_decomposition` call resolves via ADL at instantiation —
     `matrix< T, A >` associates `feng`, where the SVD template is declared
     later in the header; a comment records this). `pinv` (5321–5325, body
     `return pinverse( m );`) becomes `return matrix_details::pinv_core( m );`
     — public signature, threshold rule, and result type unchanged.
   - `random` overloads (5353, 5358) and `random_like` (5369–5372): deleted.
   - `rand_like` (5363–5367): body `return random< T, A >( row, col );` becomes
     `return rand< T, A >( row, col );`.
   - `randn_like` (5374–5379): body `return rand_like( x );` becomes
     `return rand< T, A >( x.row(), x.col() );` (was `rand_like` → `random` →
     `rand`; behavior identical; name kept, Q4).
   - free `det(m)` (4410): deleted (member `matrix::det()` at ~2057 stays).
6. **`rand` (5329) is untouched** (S5 territory; seed-0/time behavior, comments,
   and `#include <random>` all stay — the include is required by `mt19937`).

## 2. Consumers

| Consumer | Change |
|---|---|
| `examples/cases/0013_prefix.hpp:3` | `feng::random<double>( 127, 127 )` → `feng::rand<double>( 127, 127 )` (S5's 2-arg `rand` form exists; same value stream) |
| `ReadMe.md:1277` | `feng::random<double>( 127, 127 )` → `feng::rand<double>( 127, 127 )` |
| `tests/cases/pinv.hpp` (F2 refinement) | drop the "pinv == pinverse" scenario (lines 49–54), TEST_CASE "Matrix pinv/pinverse" → "Matrix pinv" (line 2), header comments (lines 4–6) reference `pinv` / `matrix_details::pinv_core` instead of `pinverse`/`svd_inverse`; the three remaining scenarios (E06 diagonal pin, 4×2 Moore-Penrose, singular behavior) untouched |

## 3. Tests

### 3.1 `tests/cases/fft.hpp` (new; registered in `tests/test.cc` between
`fabs` and `flip`, verified alphabetical)

Header comment records: R-19 suite policy (tolerances on finite values), the
oracle's provenance (corrected naive DFT, F1), and the freeze (R-18).

Note on `pinv.hpp` (F2): no tolerance-related assertions exist (the real
threshold is the hard-coded `|w| > 1.0e-10` inside the core); only the alias
scenario and naming are touched.

Oracle (self-contained, does not include matrix.hpp's private core):
`naive_fft_ref( Mat const& x, bool inverse )` — the corrected 4-loop naive DFT
with `double`-theta twiddles, exactly the structure frozen in `fft_private`.

Scenarios (fixed finite inputs only):
1. **Differential, fast path:** 8×8 `x[r][c] = sin(r·c) + 0.5·cos(0.3·r − 0.7·c)`
   (float and double): `‖fft(x) − ref(x, fwd)‖∞ < 1e-9` (double) / `< 1e-3`
   (float, vs the double-math oracle; float accumulation, R-19).
2. **Differential, fallback path:** 6×8 (row 6 = not PoT) same check, plus a
   126×128 float quick case (`‖·‖∞ < 1e-2`, magnitudes ~2.4e4 make the float
   ULP dominate; asserts the fallback ran by comparing against the ref — no
   path introspection needed).
3. **E16 (in-suite mirror of the probe):** 8×8 delta at (0,0) → `fft` all ones
   within 1e-9; `‖ifft(fft(x)) − x‖∞ < 1e-9` for the 3·ones+δ input.
4. **Normalization exactly once:** `ifft(ifft(x)) == flip2d(x)/(R·C)` within
   1e-9 — with the unscaled inverse kernel `G`, `G∘G = (R·C)·flip2d(x)` (both
   axes flip), so with per-call scale `s` the composition is `flip2d(x)·(R·C)·s²`,
   which equals the expected value only for `s = 1/(R·C)` exactly (catches
   double application on one call and missing application).
5. **E17 pins:** `fftshift`/`ifftshift` of a 1×3 row `[1 2 3]` and 3×1 column:
   value order `(1,2,0)` for both functions; 4×1/1×4 order `(2,3,0,1)` for both.
6. **Even-dim regression (C13 note):** 4×8 `fftshift` matches the pre-fix
   swap-of-halves result exactly within 1e-9 (both are the same permutation;
   bit-identity is asserted in the probe where the pre-fix binary can't be
   linked, so the suite asserts permutation equality against a hand-rolled
   swap-of-halves copy).
7. **Strides/edges:** 1×8 and 8×1 (row-only / column-only fast path) vs ref;
   1×1; complex-input `ifft` of a complex matrix vs ref.
8. **Empty guard:** `fft(matrix<float>(0, 0))` → 0×0 (no crash, matching the
   pre-fix guard).

### 3.2 `.work/probes/E16_E17.cc` (verbatim from the contract; `-O1`)

Run pre-fix at HEAD → record the failure (baseline; already characterized by
`s6_prefix_probe.log`). Run post-fix → must print `E16_E17 PASS` and exit 0.
Build: `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s6 .work/probes/E16_E17.cc && .work/probe_s6`.

### 3.3 `.work/probes/E18_a2.cc` (contract A3 compile probe)

Attempts to use `feng::random`, `feng::random_like`, `feng::pinverse`,
`feng::svd_inverse`, and free `feng::det`; the probe is built with the library
header and **must fail to compile** naming one of the retired identifiers, while a
companion `E18_ok.cc` using `feng::pinv` + `feng::rand` **must compile and run**.
Both outcomes recorded in `.work/evidence/s6_e18.log`.

### 3.4 `tests/cases/pinv.hpp` (F2)

Alias scenario removed; remaining 3 scenarios (pinv value check, tolerance
behavior, singular-matrix behavior) untouched. `make test` stays green.

## 4. ReadMe sweep (single editor: this session; ReadMe is 2391 lines at HEAD)

Ordered edits (anchors verified at HEAD):

1. **Line ~1277** (`rand` section): `random(1,2,3.0)` → `rand(1,2,3.0)`.
2. **After the `load`/`save` code block (~1063):** insert the S2 `load_npy` delta
   **verbatim** from the S2 handoff (S2 is the sole editor of that delta; quote,
   do not paraphrase).
3. **`det` section (~764–774):** add the S4 note: exact-zero pivots in
   pivoted-LU detection (the `det` doc already says "may not be accurate"; add
   the one-line exact-zero statement from the S4 handoff).
4. **SVD / `pinv` area (~2244 API line + ~1764 prose):** add (a) the SVD tuple
   order note `(u, w, v)` — S4 delta; (b) the **R-20 wide-SVD disclosure**
   (S4 watch item, S6 owns the ReadMe disclosure per the risk register): the
   SVD path is validated for `row <= col` shapes; wide matrices (`row > col`)
   are untested and the `pinv`/`svd` contract there is undocumented.
5. **`conv` section:** S4 delta — same-mode valid requires `rb >= 1 && cb >= 1`
   (1×1 kernel = pure scaling, not a crash).
6. **`rref` section (~786):** S4 delta — precondition is now
   `row > 0 && col > 0`; square matrices fully reduced; rectangular
   (`row < col`) reduced with free columns; `row > col` (over-determined)
   behaves as pre-relaxation (documented, E19 pin).
7. **Cholesky section:** S4 delta — `cholesky_decomposition` returns `bool`
   (`true` = PD factor computed; `false` = non-PD, matrix left partially
   written; strict-positivity diagonal guard `sum <= 0`).
8. **Statistics section (mean/variance/standard_deviation):** S4 delta —
   integer/float/double inputs now return `double` (promoted); sample (n−1)
   variance pins.
9. **New section `#### fft -- fast Fourier transform`** after the `lu`
   decomposition section (~1800): algorithm (radix-2 when both dims PoT else
   O(n⁴) naive fallback), complexity, NumPy convention + `ifft`
   normalization, `fftshift`/`ifftshift` convention + the fused-design
   deviation note, and the **alias-retirement table**
   (`random→rand`, `random_like→rand_like`, `pinverse→pinv`,
   `svd_inverse→pinv` (via `matrix_details::pinv_core`), `feng::det(m)→m.det()`).
10. **New short section `#### assertions and NDEBUG`** (build area, ~1364): the
    S5 C7 policy **quoted verbatim** from `docs/session_5/design.md` §4:
    assertions are debug-only, `better_assert` is a no-op under NDEBUG, public
    contracts are precondition documents not runtime guarantees (e.g. the
    `row > col` UB is reachable under NDEBUG — S4).
11. API reference block (2050–2280): no `random`/`pinverse`/`svd_inverse` rows
    exist (verified), so nothing to remove there; the block stays
    partial/illustrative (documented limitation).

## 5. Evidence and verification plan

| Evidence file | Content |
|---|---|
| `.work/evidence/s6_baseline.log` | `make test` + `./test_test` + `make example` at HEAD (green; 49,217,191 assertions / 74 cases) |
| `.work/evidence/s6_prefix_probe.log` | pre-fix fft/fftshift behavior + swap-vs-roll simulation (F1/F2 evidence) |
| `.work/evidence/s6_e16_e17.log` | probe pre-fix (FAIL baseline) + post-fix (PASS) |
| `.work/evidence/s6_e18.log` | compile probe outcomes (retired names fail, canonical names pass) |
| `.work/evidence/s6_final.log` | full `make test` + example + greps at closeout |
| `docs/evidence_map.md` | C-08 updated (the pre-fix "correct DFT" note corrected), A2 note updated (0013 + ReadMe consumers, pinv.hpp F2), new E16/E17 rows, A3 note → "S6 live" |
| `docs/eval_seed_cases.md` | E16, E17, E18 promoted (live) |

Gates (contract `exit_criteria`): E16 PASS post-fix; `fftshift` even-dim
bit-identity (probe + suite); A2/A3 grep zero; `make test` green (75 test
cases: 74 + the new `fft` case); ReadMe sweep verified against the session
checklist; handoff written; E16/E17 promoted.

## 6. Risk notes carried into the review

- Radix-2 bit-reversal for `n = 1` (1×N / N×1 and 1×1): the degenerate case
  must be a no-op pass (single element; `len` loop never runs).
- `twiddle_table` for `n = 2`: one entry `w[0] = 1`.
- Float precision on the differential oracle: the oracle computes in `double`
  internally for the float matrix (value_type promotion) to keep the tolerance
  honest; the suite's float tolerance is 1e-4.
- `-Ofast` on the test suite may reassociate the radix-2 sums differently from
  the oracle; tolerances (not bit-equality) are the gate in-suite; the probe
  runs at `-O1` without fast-math for the identity checks.
- `randn_like` re-point (Q4): zero consumers verified by grep; behavior
  identical (same seed-0 stream shape).
- R-19: no NaN-dependent assertions anywhere in the new suite code.
