# Spec: a3-verification (A3)

Scope: the A2/A3 verification pass. Pre-flight was run at session start (grep
evidence below); this spec pins the post-fix zero-state and the namespace
hygiene note.

## Pre-flight (recorded at session start, HEAD d5e7b56 — verified by grep)

- `grep -c 'pinverse\|svd_inverse' matrix.hpp` = **4**:
  5307 (`svd_inverse` def), 5317 (`pinverse` def), 5319 (`pinverse` body calls
  `svd_inverse`), 5324 (`pinv` body calls `pinverse`). All in scope for A2.
- `grep -c 'random\b' matrix.hpp` = **4**: line 29 (`#include <random>` —
  stays, required by `mt19937`), 5353 + 5358 (the two `random` overloads),
  5366 (`rand_like` body calls `random<T, A>( row, col )`). Plus the
  `random_like` identifier at 5369 (does not match `random\b` because of the
  underscore) and `randn_like`'s body calling `rand_like` (~5375). All in
  scope.
- Free `det(m)` present at line 4410. Member `matrix::det()` separate and
  retained.
- `feng::random` consumers in repo: `examples/cases/0013_prefix.hpp:3`
  (`feng::random<double>( 127, 127 )`) and `ReadMe.md:1277` (same call).
  Both canonicalized to `feng::rand<double>( 127, 127 )` in A2.
- No test file references `random`/`random_like`/`svd_inverse`; one reference
  to `pinverse`: `tests/cases/pinv.hpp:53` (the F2 refinement target), plus
  the TEST_CASE name (line 2) and header comments (lines 4–6) that mention
  `pinverse`/`svd_inverse`.

## Scenarios

#### Scenario: retired identifiers are absent post-fix
- Given the completed A2 deletions
- When the header is grepped
- Then `grep -c 'pinverse\|svd_inverse' matrix.hpp` is 0
- And `grep -n 'random\b' matrix.hpp` returns exactly one line — line 29,
  `#include <random>` (the A2 invariant keeps the include; F3: the literal
  acceptance count of 0 also matches the include line, so the gate is
  interpreted over identifier use; the match list is recorded in the
  evidence log, not just the count)
- And `std::random_access_iterator_tag` (line 151) does not match (`random_`
  has no word boundary after `random`)

#### Scenario: the free det alias is gone, the member stays
- Given the A2 deletions
- When the header is grepped and the suite runs
- Then free `feng::det(m)` no longer compiles, `m.det()` still does, and the
  S1 `det` regression test still passes

#### Scenario: namespace hygiene note is written
- Given the completed A3 pass
- When the handoff is written
- Then it states: `matrix.hpp` now exposes the canonical names only for the
  A2 families; the retired aliases are removed with no deprecation shims
  (the project's no-backward-compatibility rule); any re-introduction is a
  new sanctioned decision, not a convenience fix

#### Scenario: no dangling references remain
- Given the full post-fix tree
- When the build, suite, and example run
- Then none reference a deleted name (the build is the proof; a dangling
  reference is a compile error)
