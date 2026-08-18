# Spec — `ndebug-policy-doc` (C7): document the `NDEBUG` policy delta

### Requirement
Author the `NDEBUG` policy delta that S6 will fold into ReadMe.md. Session 5 makes
**no code and no ReadMe change**; the deliverable is the authored text below, pinned
by its appearance in `docs/session_5/specs/ndebug_policy.md`, `docs/session_5/design.md §4`,
and the S5 handoff.

### Constraints
- The mechanism claims must match source (verified P12): `debug_mode` (matrix.hpp
  57–61) is `constexpr 0` when `NDEBUG` is defined, `1` otherwise;
  `print_assertion` prints to `std::cerr` and `abort()`s only in debug mode; under
  `NDEBUG` every `better_assert` is a silent no-op.
- The delta must state the in-repo I/O-boundary precedent set by S2 (`load_npy`)
  and S5 (`save_png`): hard, NDEBUG-independent, silent-failure checks at I/O
  boundaries vs debug-only `better_assert` preconditions (S4's decomposition guards
  are control flow, not assertions).
- ReadMe.md, the `better_assert` macro body, and `debug_mode` are out of scope.

#### Scenario: authoring check
- The draft text (design.md §4) is present verbatim in this spec and in the S5
  handoff; a future-session (S6) ReadMe edit can apply it without re-deriving the
  mechanism.

### Acceptance (from contract)
- "The delta is authored and pinned in the session docs; S6 applies it to ReadMe.md."
- Verification: `grep -c 'NDEBUG' docs/session_5/design.md` ≥ 1 and the handoff
  carries the delta text (existence check; no code evidence required).

### Out of scope
Converting `better_assert` to `static_assert` (the review's suggestion — explicitly
noted as not in scope here); changing `debug_mode`; any production code.
