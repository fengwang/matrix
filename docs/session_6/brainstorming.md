# Session 6 — Brainstorming (functional-thinking / ACD)

Status: complete. Per the functional-thinking skill (existing-code + greenfield hybrid),
the new FFT units are designed with an ACD blueprint, and 2–3 approaches with trade-offs
are evaluated for each capability before the design is locked. Open questions were already
resolved in `interview.md` (one escalated item: Q1 blast-radius refinement).

## 1. ACD classification of the units S6 touches

| Unit | ACD class | Reasoning / 1000x test |
|---|---|---|
| `fft( matrix<T> const& ) → matrix<complex>` | Calculation | pure transform; no I/O, clock, or RNG (1000 identical calls → same result) |
| `ifft( matrix<complex> const& ) → matrix<complex>` | Calculation | pure transform + one deterministic `1/(R·C)` scaling |
| `fftshift` / `ifftshift` | Calculation | transform (Calculation) composed with a pure reindex |
| `fft_private::is_power_of_two( n )` | Calculation | trivial predicate |
| `fft_private::twiddle_table( n, inverse )` | Calculation | deterministic from `(n, sign)`; per-call (no cache → no hidden state) |
| `fft_private::radix2_fft_1d( buf, start, stride, n, w )` | Calculation | in-place on a **locally owned** buffer (Check 4: local-scope mutation is fine) |
| `fft_private::naive_dft( x, inverse )` | Calculation | pure O(n⁴) reference; the frozen differential oracle's provenance (F1) |
| `fftshift_private::shift_roll( x )` | Calculation | pure per-axis reindex: `new[i] = old[(i − s) mod n]`, `s = (n+1)/2` |
| `matrix_details::pinv_core( m )` | Calculation | unchanged body, relocated from the deleted `svd_inverse` (hard-coded `|w| > 1e-10` threshold; no tolerance parameter) |
| `rand`/`rand_like`/`randn_like` (1-arg forms) | **Action** (clock-seeded) | S5 boundary unchanged; seeded variants are Calculations (seeded = deterministic) |
| `lu_decomposition` / `rref` / `cholesky` / statistics | Calculation | unchanged (A2/A3 + ReadMe only touch their docs) |

**Check 2 (impurity creep):** no Action leaks into the pure core — the FFT path never reads
clocks/RNGs; the `rand` family stays at the API shell exactly as S5 left it.
**Check 4 (mutation discipline):** `fft`/`ifft`/`fftshift`/`ifftshift` never mutate the
caller's matrix (they build a local copy/buffer); the only in-place mutation is
`radix2_fft_1d` on its locally owned buffer.
**Check 5 (boolean blindness):** the radix-2 kernel takes **no `inverse` bool** — the sign
is encoded in the twiddle table (data, not a flag). The naive fallback keeps a single
documented `inverse` flag (the sign convention is the whole parameter; interface comment
states it — comment-first sentinel satisfied).
**Check 3 (explicit data flow):** every helper takes all inputs by parameter; no globals.

## 2. FFT core — approaches

### Approach A (chosen): single local buffer, strided in-place DIT radix-2, whole-matrix selection

- `fft(x)`: if `R==0 || C==0` return empty (existing guard). If **both** dims are powers of
  two: copy `x` into a local `std::vector<std::complex<T>>` (row-major); run
  `radix2_fft_1d` over each row (stride 1, len C), then over each column (stride C, len R),
  using per-size twiddle tables `wF[n] = exp(−2πi·k/n)` (forward) / `wI[n] = exp(+2πi·k/n)`
  (inverse), `k < n/2`; bit-reversal permutation first; stage loop `len = 2,4,…,n` with
  twiddle `w[k·(n/len)]`. Return the matrix built from the buffer.
  Otherwise: `naive_dft(x, inverse=false)` (whole-matrix, corrected data index).
- `ifft(x)`: same with `wI`, then scale every element by `1/(R·C)` exactly once.

Kernel indexing (derivation, checked): stage block length `len` needs
`exp(∓2πi·k/len)` for `k < len/2`; table entry `w[j] = exp(∓2πi·j/n)` gives that at
`j = k·n/len` (`< n/2` ✓). One kernel serves rows and columns via `(start, stride)`.

**Pros:** minimal new code (~120 lines incl. comments); one kernel; pure; no transposes;
the column pass is strided on a *local* buffer so no caller aliasing concerns.
**Cons:** strided column pass is less cache-friendly than contiguous-only; accepted — the
PRD target is correctness + O(n²log n) vs O(n⁴), not memory optimality (the 512×512
benchmark gate is 10–100×, which A achieves by a wide margin).

### Approach B: contiguous-only kernel with transpose between passes

Row DFT (stride 1), transpose, column DFT, transpose back.
**Rejected:** two extra O(n²) copies, two index-math code paths, transpose-bug surface —
more new risk for no correctness gain (interview Q3).

### Approach C: keep the old 4-loop naive skeleton, replace inner 1-D sums with radix-2

Gather each row into a temp, radix-2 it, scatter the result into the 2-D accumulator.
**Rejected:** awkward strided gather/scatter inside the double loop; strictly more code
than A and no benefit (it is A with the data flow inverted).

## 3. `fftshift`/`ifftshift` — approaches

### Approach A (chosen): transform, then a shared pure per-axis roll

`fftshift(x) = shift_roll( fft(x) )`, `ifftshift(x) = shift_roll( ifft(x) )`, with
`shift_roll` doing `new[i] = old[(i − s) mod n]`, `s = (n+1)/2`, per axis, on a **new**
matrix (no in-place mutation of the transform result needed — the result is already a
fresh local). Even-n behavior is bit-identical to pre-fix because swap-of-halves **is**
the roll by `n/2` (verified by the pre-flight transcription probe: n=4 both `(2,3,0,1)`).
Odd-n now matches the pinned NumPy convention (n=3 `(1,2,0)`, n=5 `(2,3,4,0,1)`).

**Pros:** one shared helper for both functions; the fused design is preserved (contract:
"the current fused transform+shift design is kept"); zero behavior change for even
dimensions; the deviation from NumPy's pure reindex is documented (C13 note).
**Cons:** the fused design stays a deliberate NumPy deviation — documented in ReadMe +
handoff (already an accepted contract decision).

### Approach B: NumPy semantics (reindex the spectrum, no transform)

**Rejected:** the contract explicitly keeps the fused design; changing semantics is a
new sanctioned decision (out of scope).

### Approach C: branch — swap for even n, roll for odd n

**Rejected:** two code paths to preserve a pin that one path already satisfies
(bit-identical); extra branch = extra surface.

## 4. Interface comments (comment-first pass) — written before implementation

- `fft`: "2-D discrete Fourier transform, NumPy `fft2` convention (unnormalized).
  Both dimensions power-of-2 → separable iterative radix-2 (O(R·C·log(R·C)));
  otherwise the O(n⁴) naive DFT (documented fallback, no new math). Pure: the input
  is not mutated. Empty input → empty output."
- `ifft`: "Inverse 2-D DFT: the same transform with conjugate kernel, followed by a
  single `1/(row·col)` normalization applied exactly once (NumPy `ifft2` convention;
  `fft` is unnormalized). Pure."
- `fftshift`/`ifftshift`: "Transform, then center the zero-frequency term: each axis is
  circularly rolled by `(n+1)/2` (NumPy convention; identical to the historical
  swap-of-halves for even n). Fused transform+shift is a deliberate NumPy deviation
  (documented); NumPy's `fftshift`/`ifftshift` are pure reindexing."
- `radix2_fft_1d`: "In-place iterative DIT radix-2 on `buf[start + i·stride]`,
  `i < n` (n a power of two). `w[k] = exp(∓2πi·k/n)` encodes the direction (no
  separate flag). Mutates only the buffer it is given."
- `naive_dft`: "O(n⁴) reference DFT; `inverse` selects the kernel sign (forward
  `exp(−…)`, inverse `exp(+…)`). Frozen after S6 as the differential oracle's
  provenance (R-18; see failure_arbiter F1: the pre-fix loop had a data-index bug and
  is NOT the oracle)."
- `matrix_details::pinv_core`: "Shared SVD pseudo-inverse core (S3 body, relocated):
  SVD out-param decomposition; singular values `w` with `|w| > 1e-10` are
  inverted, the rest set to zero; returns `v * w * uᵀ`. Hard-coded strict
  threshold (no tolerance parameter). `pinv` delegates to this."

## 5. Error strategy (define-aways pyramid)

| Operation | Failure mode | Tier | Residual contract |
|---|---|---|---|
| `fft`/`ifft` on empty (R==0 || C==0) | nothing to compute | define-away | empty result (existing behavior preserved) |
| non-power-of-2 shape | fast path inapplicable | define-away | fallback path selected (no error, documented) |
| `pinv` tolerance | user-controlled | propagate (documented param) | S3 behavior unchanged |

No new fallible operations are introduced; no try/catch surface changes (the library's
`better_assert` precondition style is untouched).

## 6. Parallelism assessment (Check 6)

The row pass and column pass are independent iterations over a heavy Calculation —
a **Data Decomposition** opportunity (parallel-for over rows/columns). **Not taken in
S6:** the build has no TBB/parallel backend (PRD goal 5 / session 11 owns
parallelization; the S6 benchmark runs serial-vs-serial as the contract states), and
adding a parallel backend now would be a new dependency — forbidden by the repo
boundaries ("do not add production dependencies without explicit approval"). Recorded as
a handoff watch item (the kernel is already decomposition-ready: embarrassingly
parallel over rows, then over columns, per call).

## 7. Data flow

```
fft(x)
  → [guard] R==0||C==0 → empty
  → is_pow2(R) && is_pow2(C)?
      yes → copy x → buf (row-major complex)
            wF[C] = twiddle_table(C, fwd)
            for r: radix2_fft_1d(buf, r·C, 1, C, wF[C])      (rows, stride 1)
            wF[R] = twiddle_table(R, fwd)
            for c: radix2_fft_1d(buf, c, C, R, wF[R])        (columns, stride C)
            → matrix from buf
      no  → naive_dft(x, fwd)                                 (corrected, whole matrix)
ifft(x)  = same with wI, then result *= 1/(R·C)               (exactly once)
fftshift(x) = shift_roll( fft(x) )        ifftshift(x) = shift_roll( ifft(x) )
A2: rand_like(x) = rand(row, col) · randn_like(x) = rand(row, col) · pinv(m) = matrix_details::pinv_core(m)
```

## 8. Test/probe design (R-19 suite policy respected)

- `tests/cases/fft.hpp` (`-Ofast` suite): tolerances on finite values only
  (1e-9 double / 1e-4 float where relevant; no NaN-dependent checks); the embedded
  oracle is a self-contained copy of the **corrected** naive DFT (frozen after S6,
  header comment records provenance + F1). Coverage: differential 8×8 (fast path) and
  6×8 (fallback path) vs oracle; E16 delta→ones + round-trip identity; E17 shift pins
  (n=3, n=4 both functions); even-dim regression pin (4×4/4×8 `fftshift` bit-identical
  to the pre-fix swap-of-halves result, value-level tolerance in-suite, exactness in
  probe); 1×8 / 8×1 strides; complex-input passthrough; normalization-once
  (`ifft(ifft(x))` carries `1/(R·C)²`, not `1/(R·C)`).
- `.work/probes/E16_E17.cc` (verbatim contract probe, `-O1`): the contract's two asserts
  exactly; run pre-fix (recorded: FAIL, baseline) and post-fix (must print PASS).
- `.work/probes/E18_a2.cc` (contract A3 compile probe): `pinv`/`rand` compile;
  `random`/`random_like`/`pinverse`/`svd_inverse`/free `det` do not (compile-fail
  expected, exit 1 with the right name in the error).
- `tests/cases/pinv.hpp`: alias-scenario removal only (F2 refinement).
- E16/E17 promoted to `docs/eval_seed_cases.md` (live); E18 promoted (live); both
  added to `docs/evidence_map.md` with the S6 closeout (row A3 "S6 live" already
  anticipated for E18).
