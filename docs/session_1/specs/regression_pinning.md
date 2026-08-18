# Spec — Capability: `regression-pinning` (new)

Authority: project contract §4 (T1/T2: "a test that would still pass with the bug present is not
a regression test"); session contract `acceptance_criteria`; PRD §7 P8.

## ADDED Requirements

### Requirement: Content-asserting regression tests for `shrink_to_size` and `flipdim`

The suite SHALL contain two new cases, `tests/cases/shrink_to_size.hpp` and
`tests/cases/flip.hpp`, registered in `tests/test.cc`:

1. Every case SHALL assert **content** (exact expected values on ragged inputs), not only shape.
2. The cases SHALL cover, at minimum, the spec scenarios of the `shrink-to-size` and `flipdim`
   capabilities (E01/E02 acceptance shapes, both non-square directions, grow-only and shrink-only
   extremes, 1×N / N×1 for `flipdim`, and the `dim==1` pin).
3. The cases SHALL FAIL if either original buggy line (`the_rows_to_copy` in the C1 copy, or
   `ans.row_begin( index_right )` in the C2 `swap_ranges`) is restored. This MUST be verified
   empirically (bug-restoration check) during the session, with output recorded.

#### Scenario: New cases are registered and green on the fixed tree

- WHEN `make test` is run on the fixed tree
- THEN the suite reports the new `shrink_to_size` and `flip` cases as passing, alongside the
  previously passing cases (no case lost).

#### Scenario: C1 bug restored → shrink case fails (bug-restoration check)

- WHEN the C1 fix line is temporarily reverted to
  `std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_rows_to_copy, other.row_begin( r ) );`
  and the shrink test case is compiled and run
- THEN the case FAILS (at least one content assertion), and after restoring the fix the case
  passes again.

#### Scenario: C2 bug restored → flip case fails (bug-restoration check)

- WHEN the C2 fix line is temporarily reverted to
  `std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.row_begin( index_right ) );`
  and the flip test case is compiled and run
- THEN the case FAILS (at least one content assertion), and after restoring the fix the case
  passes again.

### Requirement: E01/E02 ASan probes exist, pass, and are registered

The probe `.work/probes/E01_E02.cc` SHALL be buildable with
`g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1`, and when run with no arguments
SHALL exit `0` and print `PASS E01` and `PASS E02` (post-fix). It SHALL encode the E01/E02
acceptance scenarios (5×5→5×3; 3×10→5×2 ragged; 1×1→4×4 grow; 3×5 and 4×4 `flipdim(·,2)`;
3×5 `flipdim(·,1)` pin). The pre-fix run outputs (reproducing the review's findings) SHALL be
recorded in `.work/evidence/` and cited in the handoff. `docs/eval_seed_cases.md` SHALL list E01
and E02 with status `promoted` (probe exists, passes, permanent home in `tests/cases/`).

#### Scenario: Post-fix ASan probe run is clean

- WHEN the contract's deterministic probe command is executed post-fix
- THEN the process exits `0` printing `PASS E01` and `PASS E02`, with no AddressSanitizer report
  on stderr.

#### Scenario: Eval-seed corpus reflects the promoted seeds

- WHEN `docs/eval_seed_cases.md` is inspected after the session
- THEN rows E01 and E02 have status `promoted` (subsuming `live`), owner `S1`, and their probe
  paths resolve to `.work/probes/E01_E02.cc`.
