# Spec — `rand-regression-tests` (E14): `tests/cases/rand.hpp`

### Requirement
A permanent suite case `tests/cases/rand.hpp` pins the E14 invariants:
explicit-seed determinism, seed inequality, the [0,1) range for `double` and
`float` instantiations, the documented int-T all-zeros behavior, the return-type
pins, and the `noexcept` removal (the engine is allocation-backed).

### Constraints
- Full case text: `docs/session_5/design.md §5` (final code).
- House style: `feng::` qualified, `[r][col]` indexing, local bools + separate
  `REQUIRE`s (Catch v2.0.1 quirk, interview Q7), tags `[rand]`.
- Registered in `tests/test.cc` after `proj.hpp`, before the commented `remquo.hpp`
  (alphabetical) — suite 73 → 74 cases.
- No NaN-dependent assertions (R-19 fast-math: suite builds `-Ofast`; the rand
  engine produces no NaN and the case asserts ranges only).
- The case is green **pre- AND post-fix** by design (invariant pin — interview Q2);
  it is a regression net, not the C11 red.

#### Scenario: pre-fix green (regression net)
- `make test` with the case present against the old engine → all pass
  (determinism/range held pre-fix, P9).

#### Scenario: post-fix green (invariant preserved)
- `make test` after the engine swap → all pass; 74 cases.

### Acceptance (from contract)
- `grep -c 'rand' tests/cases/rand.hpp` ≥ 1 (the file exists and is registered).
- `make test` → 74 cases, all pass.

### Out of scope
Distribution-shape statistical tests; seed-0 (time-based) content checks
(non-deterministic by design); `random`/`random_like`/`randn_like` alias coverage
(S6 territory).
