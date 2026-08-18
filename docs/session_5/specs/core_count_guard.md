# Spec — `core-count-guard` (C12): clamp `hardware_concurrency()` to ≥ 1

### Requirement
Every `std::thread::hardware_concurrency()` result used as a divisor or thread count
is clamped to at least 1 before use. `hardware_concurrency()` returns 0 when the
count "could not be determined" ([thread.hard_concurrency]); an unguarded 0 is a
divide-by-zero (SIGFPE in release) at the `reduce` division (matrix.hpp:1163).

### Constraints
- Site 1 (1152, `reduce`): `unsigned int const total_cores = …` becomes
  `unsigned int total_cores = …` + `if ( total_cores < 1 ) total_cores = 1;`.
- Site 2 (4121, `reduce_impl_private`): `auto parallel_size = …` gains
  `if ( parallel_size < 1 ) parallel_size = 1;` before the existing short-circuit
  (`parallel_size <= 1 || mat.size() < 32`). Behavior-neutral (0 already took the
  sequential path) — the clamp is contract-mandated and removes reliance on the
  incidental `<= 1` (discrepancy logged: the plan called this site "unguarded";
  source shows the short-circuit guard — brainstorming P5).
- Site 3 (279): the existing `total_cores <= 1` guard is **untouched** (contract:
  leave it).
- No new test cases (contract: "no new test cases for C12 … E15 is a probe-only
  case"). `hardware_concurrency() == 0` cannot be forced in this environment
  (affinity-adjusted online count; `taskset` yields 1, not 0) → evidence = grep +
  code review.

#### Scenario: undetermined core count
- `hardware_concurrency() == 0` → `reduce` and the parallel `reduce_impl` path
  clamp to 1; no SIGFPE, no zero-thread pool.

#### Scenario: valid machines
- 1-N core machines: behavior unchanged (the clamp fires only on 0).

### Acceptance (from contract, with the logged refinement)
- `grep -c 'total_cores < 1' matrix.hpp` == 1 and `grep -c 'parallel_size < 1'
  matrix.hpp` == 1 (the literal `total_cores < 1 >= 3` is unachievable: only two
  `total_cores`-style sites exist and 279 keeps its `<= 1` — refinement logged in
  interview Q6).
- `make test` green.

### Out of scope
Thread-pool sizing heuristics (the `thread_count` formulas), the `mat.size() < 32`
threshold, any other `hardware_concurrency()` consumer (audit: exactly three sites,
brainstorming P3).
