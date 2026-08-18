# Spec — `rand-engine` (C11): `rand` uses a per-call local engine

### Requirement
`feng::rand` (matrix.hpp:5323) fills its result from a **per-call local**
`std::mt19937` engine and `std::uniform_real_distribution<T>(0.0, 1.0)` — no
`srand`, no `std::rand`, no `RAND_MAX`, no global or thread-local generator state.
`noexcept` is removed from the whole rand chain (5323/5355/5361/5366).

### Constraints
- Seed 0 keeps the exact pre-fix mix:
  `static_cast<unsigned int>( time + reinterpret_cast<uint_least64_t>(&ans) )`
  (time-based per the contract's R-05 note; the residual per-call-site/same-second
  correlation is inherent to this seed and is documented, not violated).
- Non-zero seeds are passed verbatim to `mt19937` (deterministic per platform/libstdc++).
- Body signature and return type unchanged: `matrix<T,A> const rand(uint_least64_t r, uint_least64_t c, unsigned int seed = 0)`;
  `rand(n)`/`random`/`random(n)` overloads and bodies untouched.
- Alias *renames* (`random`/`random_like`/`randn_like` → S6) untouched; only their
  `noexcept` specifiers change (interview Q3 — same-defect extension, boundary documented).
- No new includes (`<random>` already at line 29).
- Complex-T `rand` no longer compiles (contract-prescribed distribution type; zero
  in-repo consumers) — documented in handoff + risk register, not fixed.

#### Scenario: deterministic explicit seeds (E14)
- `rand<double>(64,64,7)` called twice → identical matrices; vs seed 8 → different.

#### Scenario: range
- All elements of `rand<double>(64,64,7)` and `rand<float>(32,32,7)` lie in [0,1).

#### Scenario: instantiation classes
- `T = int` → all elements 0 (documented; unchanged from pre-fix integer division).
- `T = double`/`float` → return type `matrix<T> const` (static_assert pinned).
- `rand` is not `noexcept` (static_assert pinned).

#### Scenario: thread safety
- Concurrent `rand` calls from two threads introduce no data race (no shared mutable
  state in user code); TSan post-fix run clean. (Pre-fix TSan clean is a documented
  limitation — the race lived in uninstrumented libc state; contract fallback clause:
  reasoning + grep for absence of global state.)

### Acceptance (from contract, with the logged grep refinements)
1. `grep -cE 'srand\(|std::rand\(' matrix.hpp` == 0 (pre-fix = 3; the literal
   `srand\|std::rand` pattern false-positives on `std::random_access_iterator_tag`
   at line 151 — refinement logged in interview Q6).
2. `tests/cases/rand.hpp` (E14) passes — green pre- AND post-fix (invariant pin).
3. TSan post-fix probe clean.
4. `make test` green (74 cases).

### Out of scope
`random`/`random_like`/`randn_like` bodies and renames (S6); seed-0 entropy
improvement beyond keeping the documented time-based mix; complex-T support;
distribution quality beyond the contract-prescribed engine.
