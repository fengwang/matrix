# Spec: alias-retirement (A2)

Scope: delete the legacy aliases from `matrix.hpp`, relocate the shared SVD
pseudo-inverse core, re-point the like-family to `rand`, update the two
in-repo consumers, and canonicalize the S3 test file (F2 refinement,
reported per project contract §1.2).

## Scenarios

#### Scenario: retired names are gone from the header
- Given the A2 deletions
- When the header is grepped
- Then `grep -c 'pinverse\|svd_inverse' matrix.hpp` is 0
- And `grep -c 'random\b' matrix.hpp` is 0 (excluding the documented
  `#include <random>` and prose; `std::random_access_iterator_tag` is not a
  match because of the `\b` boundary)
- And free `feng::det(m)` is gone (the member `matrix::det()` stays)

#### Scenario: canonical names survive with unchanged signatures
- Given the deletions
- When `feng::pinv(m)` and `feng::rand(...)` (all three S5 forms) are called
- Then they compile and behave exactly as before (S3/S5 behavior unchanged;
  `pinv` now delegates to `matrix_details::pinv_core`)

#### Scenario: the pinv core lives in matrix_details
- Given the SVD pseudo-inverse body (the S3 `svd_inverse` loop)
- When `pinv` is called
- Then the computation runs through `matrix_details::pinv_core` (one shared
  core; threshold unchanged: a singular value `w` is inverted iff `|w| >
  1.0e-10` — hard-coded and strict, no tolerance parameter is added or
  removed)
- And the result type and public signature are unchanged (`pinv(m)` for a
  `matrix<T,A>` returns `matrix<T,A>`)

#### Scenario: the like-family is re-pointed, not dangling
- Given `rand_like` and `randn_like` after the `random_like` deletion
- When either is called on a matrix `x`
- Then it returns `rand< T, A >( x.row(), x.col() )` (the S5 two-arg seed-0
  form; behavior identical to the pre-fix chain)
- And no reference to a deleted name remains (adversarial case: A2 leaving no
  dangling references)

#### Scenario: consumers are canonical
- Given the in-repo consumers
- When the build and suite run
- Then `examples/cases/0013_prefix.hpp` uses `feng::rand`, `ReadMe.md`'s rand
  example uses `rand(1,2,3.0)`, and `tests/cases/pinv.hpp` uses only
  `feng::pinv` (the alias-equivalence scenario is removed — F2; TEST_CASE
  renamed to "Matrix pinv")

#### Scenario: the compile probe passes
- Given `.work/probes/E18_a2.cc`
- When it is compiled against the post-fix header
- Then it fails to compile, naming one of the retired identifiers
- And the companion probe using `feng::pinv` + `feng::rand` compiles and runs
  (outcome logged in `.work/evidence/s6_e18.log`)
