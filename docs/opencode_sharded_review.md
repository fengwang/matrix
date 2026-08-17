# Sharded Code Review — `matrix.hpp`

- **Date:** 2026-07-13
- **Scope:** `matrix.hpp` (7,689 lines, single-header C++20 matrix library, `namespace feng`), plus `tests/` and `ReadMe.md` for contract evidence.
- **Method:** Manual review along six axes (correctness, readability, security/safety, tests, architecture, performance) followed by empirical verification with GCC 16.2 (C++20, `-DPARALLEL`):
  - Full test suite built via `make test` and executed: **All tests passed (49,216,592 assertions in 57 test cases)**.
  - AddressSanitizer probes compiled with `-DNDEBUG -DPARALLEL -fsanitize=address` (so `better_assert` is a silent no-op and the *actual* out-of-bounds behavior is observable rather than aborted on a precondition).
- **Line numbers** refer to `matrix.hpp` at review time.

## Findings summary

| # | Severity | Axis | Finding | Evidence verified |
|---|----------|------|---------|-------------------|
| C1 | **Critical** | Correctness | `shrink_to_size` copies the wrong column count → heap OOB write + silent corruption | ASan-confirmed |
| C2 | **Critical** | Correctness | `flipdim(m, 2)` swaps a *column* with a *row* → heap OOB (non-square) / silent corruption (square) | ASan-confirmed |
| C3 | High | Correctness | `fliplr`/`flipud` aliases are swapped vs. conventional semantics | Code-verified |
| C4 | High | Correctness | `pinverse`/`pinv` never inverts the singular values | Probe-confirmed (returns 2.0 where 0.5 expected) |
| C5 | High | Correctness | `det()` Schur complement uses `P.inverse()` with no singularity handling → silent `NaN` | Probe-confirmed |
| C6 | High | Correctness | `operator^` does not compile for any odd exponent ≥ 3 (precedence bug) | Compile probe-confirmed |
| S1 | High | Security/Correctness | `load_npy` performs no buffer-size validation → OOB read on truncated files; `stoul` can throw from `noexcept` | ASan-confirmed |
| P1 | High | Performance | `fft`/`ifft` are naive O(N⁴) direct DFTs despite the FFT name | Code-verified |
| S2 | Medium | Security/Safety | `save_png` dereferences unchecked `fopen` result (null `FILE*`) | Code-verified |
| C7 | Medium | Correctness | `better_assert` silently no-ops under `NDEBUG`, turning all boundary checks into UB paths in release builds | Code-verified |
| C8 | Medium | Correctness | `mean`/`variance`/`standard_deviation` truncate for integer matrices | Probe-confirmed |
| C9 | Medium | Correctness | `conv` "same" mode: second assert checks `rb` instead of `cb`; both reject valid 1×1 kernel | Code-verified |
| C10 | Medium | Correctness | `rref`/`gauss_jordan_elimination` precondition `row < col` rejects square systems the algorithm handles | Code-verified |
| C11 | Medium | Correctness | `rand` uses global `srand`/`rand`: re-seeds every call, not thread-safe, low quality | Code-verified |
| T1 | Medium | Tests | Test suite is green but the five most buggy code paths (shrink_to_size, flipdim, pinverse, det, `^`) have zero test coverage | Verified by listing `tests/cases/` |
| A1 | Medium | Architecture | ~30 CRTP mixins each re-derive identical typedefs via `type_proxy_type`; high indirection for a single concrete class | Code-verified |
| R1 | Medium | Readability | `svd_inverse` calls `singular_value_decomposition(a, u, v, w)` with swapped argument order vs. the signature `(a, u, w, v)` | Code-verified |
| R2 | Low | Readability | ~40 nearly identical 6-line elementwise templates (unary/binary/complex math, ~1,200 lines of boilerplate) | Code-verified |
| R3 | Low | Readability | Stray double semicolon in `save_png`; typo in `det` precondition message ("the row and matrix are supposed to be same") | Code-verified |
| C12 | Low | Correctness | `matrix_details::reduce` divides by `hardware_concurrency()` which may be 0 → SIGFPE | Code-verified (unreachable on typical hosts) |
| A2 | Low | Architecture | API duplication: `random`↔`rand`, `random_like`↔`rand_like`, `pinv`↔`pinverse`; free `det(m)` + member `m.det()` | Code-verified |
| P2 | Low | Performance | `lu_decomposition` has no partial pivoting (stability), and `cholesky` has no positive-definiteness guard | Code-verified |
| C13 | Low | Correctness | `fftshift`/`ifftshift` are wrong for odd dimensions (pair-swap, not circular rotation) | Derived (not executed) |

---

## Correctness

### C1 — `shrink_to_size` copies the wrong column count (Critical)

- **Severity:** Critical (memory corruption)
- **Evidence:** `matrix.hpp:3528-3532`
  ```cpp
  size_type const the_rows_to_copy = std::min( zen.row(), new_row );
  size_type const the_cols_to_copy = std::min( zen.col(), new_col );

  for ( size_type r = 0; r != the_rows_to_copy; ++r )
      std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_rows_to_copy, other.row_begin( r ) );
  ```
  The loop copies `the_rows_to_copy` **columns per row** instead of `the_cols_to_copy`.
- **Violated contract:** the documented behavior ("if new row or col are larger than the original, padding with zero; otherwise, drop these elements", comment at `matrix.hpp:3515-3517`).
- **Impact (empirically verified):**
  - `matrix<double>{5,5,1.0}.shrink_to_size(5,3)` → AddressSanitizer: `heap-buffer-overflow` at `matrix.hpp:3532`.
  - `matrix<double>{3,10}.shrink_to_size(5,2)` → no crash but **silent corruption**: last row becomes `(21, 22, 23)` instead of the documented zero padding.
- **Smallest safe fix:** `std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_cols_to_copy, other.row_begin( r ) );`
- **Confidence:** 100% (reproduced).

### C2 — `flipdim(m, 2)` swaps a column with a row (Critical)

- **Severity:** Critical (memory corruption)
- **Evidence:** `matrix.hpp:4476-4481`
  ```cpp
  std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.row_begin( index_right ) );
  ```
  The third argument of `swap_ranges` must be the start of the *second column*, i.e. `ans.col_begin( index_right )`. As written, it swaps a column (length `row()`) against a *row* (length `col()`).
- **Violated contract:** `flipdim` must flip along dimension 2 (left/right flip), per the parallel structure of the `dim == 1` branch and the public `fliplr`/`flipud` API.
- **Impact (empirically verified):**
  - Square 4×4: result **does not equal** a left-right flip (silent data corruption).
  - Non-square 3×5: AddressSanitizer `heap-buffer-overflow` at `matrix.hpp:4479`.
- **Smallest safe fix:** use `ans.col_begin( index_right )` as the third argument.
- **Confidence:** 100% (reproduced).

### C3 — `fliplr` / `flipud` aliases are swapped (High)

- **Severity:** High (wrong semantics; compounds C2)
- **Evidence:** `matrix.hpp:4491-4499`
  ```cpp
  matrix<T,A> const fliplr( matrix<T,A> const& m ) { return flipdim( m, 1 ); }  // dim 1 flips up/down
  matrix<T,A> const flipud( matrix<T,A> const& m ) { return flipdim( m, 2 ); }  // dim 2 flips left/right
  ```
- **Violated contract:** MATLAB/NumPy convention, which this library follows elsewhere (`meshgrid`, `conv`, pooling): `fliplr` = left-right (column) flip, `flipud` = up-down (row) flip.
- **Impact:** users get the transpose-axis flip they didn't ask for; silent, no error.
- **Smallest safe fix:** `fliplr → flipdim(m, 2)`, `flipud → flipdim(m, 1)`.
- **Confidence:** High (semantics by convention; the flipdim body itself is broken anyway).

### C4 — `pinverse` / `pinv` never inverts the singular values (High)

- **Severity:** High (silently wrong numerical results)
- **Evidence:** `matrix.hpp:5226-5230`
  ```cpp
  Matrix const pinverse( const Matrix& m )
  {
      Matrix u, w, v;
      singular_value_decomposition( m, u, w, v );
      return v * w * u.transpose();          // W is the diagonal of singular values, NOT inverted
  }
  ```
  The pseudoinverse is `V · Σ⁺ · Uᵀ`; this returns `V · Σ · Uᵀ`. Compare `svd_inverse` (`matrix.hpp:5216-5224`), which *does* invert the diagonal with a 1e-10 threshold and produces correct results.
- **Violated contract:** a function named `pinverse` must compute the Moore–Penrose pseudoinverse.
- **Impact (empirically verified):** `pinverse(diag(1,2))` returns `diag(1, 2)`; expected `diag(1, 0.5)`. `svd_inverse(diag(1,2))` correctly returns `diag(1, 0.5)`.
- **Smallest safe fix:** `return svd_inverse( m );` (delete the body), or apply the same diagonal-inversion loop as `svd_inverse`.
- **Confidence:** 100% (reproduced).

### C5 — `det()` uses `P.inverse()` without handling a singular P (High)

- **Severity:** High (silent `NaN`/wrong results)
- **Evidence:** `matrix.hpp:2063-2067`
  ```cpp
  zen_type const& tmp = S - ( R * ( P.inverse() ) * Q );
  return P.det() * tmp.det();
  ```
  The Schur-complement identity `det = det(P)·det(S − R·P⁻¹·Q)` requires `P` nonsingular. There is no check; `inverse()` on a singular block yields `inf`/`NaN`, which propagates silently.
- **Violated contract:** `det` must return the determinant for any square matrix (ReadMe §"det -- matrix determinant"); for singular input the answer is `0`, not `NaN`.
- **Impact (empirically verified):**
  ```cpp
  // P block [[1,2],[2,4]] is singular; true determinant is 0
  det(m) == -nan
  ```
- **Additional evidence:** the precondition message at `matrix.hpp:2056` has a typo ("the row and matrix are supposed to be same").
- **Smallest safe fix:** compute the determinant via the existing `lu_decomposition` (`matrix.hpp:~6700`): `det = ±∏U_ii` with a singularity check, and return `NaN`/`std::optional` on pivot zero. This also removes the O(n³)-per-level `inverse()` (see P2).
- **Confidence:** 100% (reproduced).

### C6 — `operator^` does not compile for odd exponents ≥ 3 (High)

- **Severity:** High (public API member unusable)
- **Evidence:** `matrix.hpp:5567`
  ```cpp
  if ( n & 1 )
      return lhs ^ ( n - 1 ) * lhs;   // parses as lhs ^ ((n-1) * lhs) — `*` binds tighter than `^`
  ```
- **Violated contract:** `m ^ n` (integer power) is a documented public operation.
- **Impact (empirically verified):**
  ```
  matrix.hpp:5567:36: error: no match for ‘operator*’
    (operand types are ‘uint_least64_t’ and ‘const feng::matrix<double>’)
        return lhs ^ ( n - 1 ) * lhs;
  ```
  `m ^ 3` fails to instantiate; only `n == 0, 1` and even powers compile.
- **Smallest safe fix:**
  ```cpp
  auto const& half = lhs ^ ( n >> 1 );
  return half * half * lhs;
  ```
- **Confidence:** 100% (reproduced).

### C7 — `mean`/`variance`/`standard_deviation` truncate for integer matrices (Medium)

- **Severity:** Medium (wrong numerical results for integer types)
- **Evidence:** `matrix.hpp:7640-7641`
  ```cpp
  auto mean( Mat const& m ) { return sum( m ) / m.size(); }
  ```
  For `matrix<int>` this is integer division.
- **Violated contract:** "mean" is the arithmetic mean; ReadMe documents `mean` for numeric matrices generally.
- **Impact (empirically verified):** `mean(matrix<int>{1,2, {1,2}}) == 1` (expected 1.5). Also `variance` of `{1,2}` is `0.25 → 0`, so `standard_deviation` of a 2-element int matrix is `0`.
- **Smallest safe fix:** promote the divisor/accumulator to `double` (or the matrix's floating-point promotion type) in the reduce helpers, or document integer truncation explicitly in the ReadMe.
- **Confidence:** 100% (reproduced); severity is a contract judgment.

### C9 — `conv` "same" mode asserts are wrong (Medium)

- **Severity:** Medium
- **Evidence:** `matrix.hpp:6620-6621`
  ```cpp
  better_assert( rb > 1, " ... the row of the second matrix is at least 1, but now has ", rb );
  better_assert( rb > 1, " ... the column of the second matrix is at least 1, but now has ", cb );
  ```
  Two problems: (1) the second assert re-checks `rb` instead of `cb`; (2) the message says "at least 1" but the condition `> 1` rejects a valid 1×1 kernel (for which "same" mode is well-defined and the code below handles it: `(rb-1)>>1 == 0`).
- **Violated contract:** the documented "same" mode (matches NumPy/Matlab `conv(...,'same')`, ReadMe §pooling/conv region).
- **Impact:** in debug builds a valid 1×1-kernel "same" convolution aborts; in release builds the column bound is never enforced.
- **Smallest safe fix:** `better_assert( rb >= 1 && cb >= 1, ... )` (or drop, since the slicing below already requires positive dims), and fix the copy-pasted condition.
- **Confidence:** High.

### C10 — `rref`/`gauss_jordan_elimination` precondition `row < col` (Medium)

- **Severity:** Medium (overly restrictive documented precondition)
- **Evidence:** `matrix.hpp:6396`
  ```cpp
  better_assert( row < col && "matrix row must be less than colum to execut a Gauss-Jordan Elimination" );
  ```
  The algorithm (partial-pivoting Gauss–Jordan, `matrix.hpp:6398-6420`) is fully defined for square and even over-determined systems; only the assert is restrictive. In debug builds `rref(square)` aborts; in release the same call succeeds — inconsistent behavior across build modes.
- **Violated contract:** `rref` (Matlab alias, comment at `matrix.hpp:6427`) is expected to work on square systems.
- **Smallest safe fix:** relax to `row > 0 && col > 0`; keep the pivot-magnitude early exit (`1.0e-10`) as the singularity signal.
- **Confidence:** High (code-level; not executed in debug mode to avoid the intended abort).

### C11 — `rand` uses the global `srand`/`rand` (Medium)

- **Severity:** Medium
- **Evidence:** `matrix.hpp:5244-5250`
  ```cpp
  if ( 0 == seed )
      std::srand( static_cast< unsigned int >( ... std::time(nullptr) + reinterpret_cast<...>( &ans ) ) );
  else
      std::srand( seed );
  auto const& generator = []() noexcept
  { return ( static_cast<T>( std::rand() ) + 1 ) / ( static_cast<T>( RAND_MAX ) + 2 ); };
  ```
- **Violated contract / invariant:** `rand` is a public API of a library whose own algorithms run on multiple threads; C++11+ `rand()`/`srand()` are not required to be thread-safe (concurrent `rand()` calls are a data race → UB), and re-seeding the single global generator from a time+address value on every call makes repeated calls within the same second highly correlated.
- **Impact:** low-quality, potentially correlated randomness; UB if users fill matrices concurrently (e.g., inside a `std::async`/thread pool).
- **Smallest safe fix:** use a local `std::mt19937` (seeded as today) and `std::uniform_real_distribution<T>(0.0, 1.0)`; drop `noexcept` if the allocation can throw.
- **Confidence:** High (code-level; concurrency impact is latent).

### C12 — `reduce` divides by `hardware_concurrency()` which may be 0 (Low)

- **Severity:** Low
- **Evidence:** `matrix.hpp:1152-1161` — `cache.resize( total_cores ); auto block_size = total_elements / total_cores;` with `total_cores = std::thread::hardware_concurrency()`, which is permitted to return `0` ("cannot determine").
- **Impact:** integer division by zero (SIGFPE) on hosts where it returns 0. The `parallel` helper at `matrix.hpp:276` guards with `total_cores <= 1`; this `reduce` path does not.
- **Smallest safe fix:** `if ( total_cores < 1 ) total_cores = 1;` (also applies to `matrix.hpp:4036`).
- **Confidence:** High.

### C13 — `fftshift`/`ifftshift` wrong for odd dimensions (Low)

- **Severity:** Low
- **Evidence:** `matrix.hpp:6340-6355` (and mirror at 6470-6485). For odd `R`, `row_starter = R/2 + 1` and the loop swaps rows `i` with `R/2+1+i` only, leaving the middle row fixed — a pair-swap, not the circular rotation by `floor(R/2)` that `fftshift` is defined as. E.g. `R=3`: produces `[2,1,0]` instead of `[1,2,0]`.
- **Smallest safe fix:** implement as a two-block move (`std::rotate` of row indices), or `row r → (r + (R-1)>>1) % R`.
- **Confidence:** Medium (derived by hand; not executed because the DFT around it makes a probe slow).

---

## Security / Safety

### S1 — `load_npy` performs no size validation on untrusted file input (High)

- **Severity:** High (out-of-bounds reads on malformed external input)
- **Evidence:** `matrix.hpp:2508-2560`. After `std::ifstream` succeeds the code dereferences fixed offsets with no length checks:
  - `buffer.data()+6` (version), `buffer.data()+8..11` (header length), `buffer.data()+10/12 + header_length` (header string), and finally `std::copy_n( buffer.data()+data_offset, row*col, ... )` where `row*col` comes from *parsed file contents*.
- **Violated contract / invariant:** "data from external sources is treated as untrusted; external data flows are validated at system boundaries before use." `load_npy` is a file-input boundary.
- **Impact (empirically verified):** a 3-byte file → AddressSanitizer `heap-buffer-overflow` read at `matrix.hpp:2520`. Additional issues:
  - `std::stoul` on a malformed header **throws** from a `noexcept` member → `std::terminate`.
  - No `dtype` check: a `float32`/complex `.npy` loaded into `matrix<double>` silently copies misinterpreted bytes.
  - In `NDEBUG` builds the only guard (`better_assert( ifs, ... )`) is a no-op, so even open failures fall through into the OOB path.
- **Smallest safe fix:** validate before any dereference:
  ```cpp
  if ( buffer.size() < 12 ) return false;
  // after parsing header_length:
  if ( buffer.size() < data_offset + header_length + std::size_t{row} * col * sizeof( value_type ) )
      return false;
  // after parsing dtype:
  if ( header.find( expected_dtype_string ) == std::string::npos ) return false;
  ```
  and either drop `noexcept` or catch `stoul` exceptions.
- **Confidence:** 100% (OOB reproduced); dtype issue verified by reading the code.

### S2 — `save_png` dereferences unchecked `fopen` result (Medium)

- **Severity:** Medium
- **Evidence:** `matrix.hpp:3100-3105`
  ```cpp
  FILE* fp = fopen( file_name, "wb" );
  for ( i = 0; i < 8; i++ )
      fputc( ( "\x89PNG\r\n\32\n" )[i], fp );;      // also a stray double semicolon
  ```
  No `if ( !fp )` check before the first `fputc` (null-pointer UB on open failure, e.g. bad path/permissions); the function is `noexcept`.
- **Smallest safe fix:** `if ( !fp ) return;` immediately after `fopen`; remove the stray `;`. Contrast with `save_as_bmp` (`matrix.hpp:~6830`), which correctly checks the stream and reports via `better_assert`.
- **Confidence:** High.

---

## Readability / Simplicity

### R1 — `svd_inverse` swaps argument order against the function signature (Medium)

- **Severity:** Medium (comprehensibility trap; one of the direct causes of C4)
- **Evidence:** `matrix.hpp:5216-5224` calls `singular_value_decomposition( a, u, v, w )` while the signature is `( A, u, w, v )`. Local variable names then match the *call site*, not the function's parameters, so reading the body (`for_each( v.begin(), ... ) 1.0/val`) requires knowing the swap.
- **Smallest safe fix:** keep names consistent with the signature: `matrix<T,A> u, w, v; singular_value_decomposition( a, u, w, v ); ... invert w ...; return v * w.transpose()*... ` (i.e., stop transposing the V matrix into the "w" slot), or better, delete `svd_inverse` and fix `pinverse` (C4) to be the single correct implementation.
- **Confidence:** High.

### R2 — ~40 near-identical elementwise templates (Low–Medium)

- **Severity:** Low (no correctness impact; maintainability cost)
- **Evidence:** the "unary functions" block (`matrix.hpp:6495-6930`) and "binary functions" block (`matrix.hpp:6940-7545`) contain ~40 functions that are the same 6-line shape: `zeros_like` + `matrix_details::for_each` + `std::<op>`. E.g. `exp`, `exp2`, `expm1`, `log`, `log10`, `log1p`, `log2`, `sqrt`, … `abs`, `exp`, `imag` each differ only in the standard function and (for complex) the result type.
- **Contract clause:** "Could this be done in fewer lines?" — this is ~1,200 lines of copy-paste.
- **Smallest safe fix:** one macro or a small `apply_unary<F>(m)`/`apply_binary<F>(a,b)` helper; or keep the explicit list but generate it via a single macro that lists the function names. Low urgency.
- **Confidence:** High.

### R3 — Dead/broken artifacts (Low)

- Stray `;;` at `matrix.hpp:3105` (save_png).
- `better_assert` typo in `det` message at `matrix.hpp:2056`: "the row and matrix are supposed to be same".
- `matrix<T,A> const` return type (top-level `const` on returned prvalues) is used across the free-function API (e.g. `magic`, `flipdim`, `rand`); harmless but non-idiomatic and signals confusion with `const&` returns.
- **Confidence:** High.

---

## Tests

### T1 — Test suite is green, but coverage avoids the buggy paths (Medium)

- **Severity:** Medium
- **Evidence:** `tests/cases/` contains 59 small files (1,158 lines total), dominated by elementwise unary-math cases (`sin.hpp`, `cos.hpp`, …). There is **no test** for: `shrink_to_size`, `flipdim`/`fliplr`/`flipud`, `pinverse`/`svd_inverse`, `det` (member or free), `operator^`/`pow` on matrices, `operator*(valarray, matrix)`, `conv` modes, `fft`, or file save/load except a single happy-path `load_npy`. Examples (`examples/cases/0005_det.hpp`, `0018_conv.hpp`, `0021_singular_value_decomposition.hpp`, …) exercise some of these, but the maintained Catch2 suite does not.
- **Violated contract clause:** "Are all error paths covered? Do the tests actually assert the right things?" — every Critical/High finding above (C1, C2, C4, C5, C6, S1) is in a path with no test, which is why a fully green suite (57 cases / 49.2M assertions) coexists with heap corruption.
- **Smallest safe fix:** add one regression case each: `shrink_to_size(5,5→5,3)` content+shape check; `flipdim` on 3×5 vs. expected; `pinverse(diag(1,2))` ≈ `diag(1,0.5)`; `det` of the singular-P matrix ≈ 0; `m ^ 3` vs. `m*m*m`; `load_npy` on a truncated file expecting `false` (needs S1 fix first).
- **Confidence:** High (file listing + `make test` run).

### T2 — Happy-path-only assertions; error paths untested (Medium)

- **Severity:** Medium
- **Evidence:** e.g. `tests/cases/ones.hpp` (shown above) only checks well-formed shapes; `load_npy.hpp` loads a valid file; no test expects `{}` from `lu_solver` on a singular matrix or `nullopt` from `gauss_jordan_elimination`.
- **Smallest safe fix:** after fixing S1/C5/C10, add negative-path cases (singular det, singular LU, truncated npy, `rref` on a square matrix).
- **Confidence:** High.

---

## Architecture

### A1 — CRTP mixin sprawl for a single concrete class (Medium)

- **Severity:** Medium (design debt; no behavior bug)
- **Evidence:** `matrix.hpp:3760` — `matrix<Type,Alloc>` inherits ~30 `crtp_*` structs (`crtp_typedef`, `crtp_inverse`, `crtp_det`, `crtp_clone`, `crtp_shrink_to_size`, `crtp_load_npy`, …). Every mixin re-derives the same typedefs through `crtp_typedef`/`type_proxy_type` and casts back with `static_cast<zen_type&>(*this)`.
- **Contract clause:** "Are abstractions earning their complexity?" — CRTP pays a real comprehension cost (a reader must jump mixin → typedef → cast to see what a method does) but buys no reuse: there is exactly one class template, and no second derived type exists. Regular member functions (grouped in sections) would delete the `zen`/`zen_type` indirection layer entirely.
- **Caveat:** this is a *refactor* recommendation, not a fix; do it after the correctness fixes land and are covered by tests (T1).
- **Confidence:** High (structural observation).

### A2 — Duplicated public API (Low)

- **Evidence:** `random`→`rand` (`matrix.hpp:5260-5268`), `random_like`→`rand_like` (`5276-5280`), `pinv`→`pinverse` (`5232-5236`), free `det(m)`→`m.det()` (`4319-4321`). Each alias is one line, but doubling the surface means every fix must be applied/verified twice (C4 shows the two SVD-inversion paths already diverged).
- **Smallest safe fix:** keep one canonical name per operation; delete or `static_assert` the duplicates.
- **Confidence:** High.

### A3 — Hostile-to-ADL name collisions (Low)

- **Evidence:** `namespace feng` defines free `abs`, `exp`, `sqrt`, `log`, `pow`, `norm`, `real`, `imag`, `conj`, `det`, `diag`, `fft`, `meshgrid` (e.g. `matrix.hpp:6505`, `7560-7630`). With `using namespace feng;` in a translation unit that also uses `std::` or third-party code, overload sets merge and unqualified calls can change meaning (e.g. `abs(x)` for a scalar now also sees `feng::abs(Mat)` — usually SFINAE'd away, but `norm` has *both* a complex-matrix version and the commented-out scalar version at `6160-6190`, which shows the drift risk).
- **Smallest safe fix:** namespace the elementwise layer (e.g. `feng::elem::`) or rename the colliding few (`norm` → `cmplx_norm`).
- **Confidence:** Medium.

---

## Performance

### P1 — `fft` / `ifft` are naive O(N⁴) direct DFTs (High)

- **Severity:** High (misleading complexity; unusable for real image sizes)
- **Evidence:** `matrix.hpp:6313-6335` (and `ifft` at `6446-6468`): quadruple-nested loops with the definition `X[r][c] = Σ_r' Σ_c' x[r'][c'] · ω…`, i.e. O(R²C²) per output element → O(R⁴C⁴)-ish per matrix, plus two `cos`/`sin` evaluations (`make_omege`) per multiply.
- **Violated contract / invariant:** the name (`fft`, and `fftshift` matching the FFT convention) implies O(N log N) behavior; a 256×256 input costs trillions of operations here.
- **Smallest safe fix:** (a) rename to `dft` and document the complexity, or (b) implement a real radix-2 FFT row-wise + column-wise (the standard separable 2-D FFT) and keep trig precomputation per row.
- **Confidence:** High (algorithm is plainly the direct sum).

### P2 — `det`/`inverse`-level routines avoid the library's own LU (Low–Medium)

- **Severity:** Low–Medium
- **Evidence:** `det` recurses through Schur complements built with `P.inverse()` (`matrix.hpp:2063-2067`), i.e. O(n³) work per recursion level instead of O(n³/3) once via the existing `lu_decomposition` (`matrix.hpp:~6700`). `lu_decomposition` itself performs no partial pivoting, so stability depends on the input; `forward_substitution` masks failure with an `isinf`/`isnan` check (`matrix.hpp:6379-6383`), and `cholesky_decomposition` has no positive-definiteness guard (sqrt of a negative silently yields `NaN`).
- **Smallest safe fix:** implement `det` via LU (folds into C5); add pivot selection to `lu_decomposition`; return `std::optional` from `cholesky` on `sum < 0`.
- **Confidence:** High.

---

## Verified non-issues (checked and found acceptable)

- `load_bmp` validates header/size consistency before parsing (`matrix.hpp:6760-6770`) — good boundary handling; the model S1 should follow.
- `save_as_bmp` checks stream construction and shape equality of the three channels.
- `expm` scaling matches the standard `A/s2` reduction (the `s == 0` case reduces to the identity scaling); the only edge is `1 << s` at `s ≥ 64`, unreachable in practice for double inputs.
- `conv` padding and `mode == "full"` path are correct; only the `"same"` asserts are wrong (C9).
- `pooling` correctly ignores leftover rows/cols (`row/dim_r` truncation) and validates the action name.
- Full test suite passes as-is (`make test`, 57 cases, 49,216,592 assertions); `examples/` builds the remaining 2 cases gated behind missing optional data.

## Suggested fix order

1. C1, C2 (memory corruption, one-line fixes each) + T1 regression tests.
2. S1 (`load_npy` validation) — unblocks negative-path tests.
3. C4 (make `pinverse` = `svd_inverse`), C5 (`det` via LU), C6 (parenthesize `operator^`), C3 (swap aliases).
4. S2, C7 (document or convert the `NDEBUG` policy), C8–C11.
5. P1 (FFT rename or real implementation), then A1/A2/R2 refactors behind the new tests.
