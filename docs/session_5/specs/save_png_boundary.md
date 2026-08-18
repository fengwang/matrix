# Spec — `save-png-boundary` (S2-finding / R3-slice): `save_png` `fopen` guard + stray `;;`

### Requirement
The free helper `save_png` (matrix.hpp:3181) must not dereference a null `FILE*`:
the `fopen` result is checked and the function returns silently on open failure.
The stray double semicolon on the PNG-signature `fputc` (line 3190) is removed
(sanctioned R3-slice).

### Constraints
- Guard: `if ( ! fp ) return;` immediately after the `fopen` (house spacing:
  `if ( ! fp )`).
- Silent no-op on open failure (R-05 note; matches the `load_npy` S2 I/O-boundary
  precedent — hard but silent, no throw, no stderr). `noexcept` on the free helper
  kept (no allocation in the function).
- The member `save_as_png` (call site 3469) is **untouched**: its `bool` return
  stays `true` on open failure (the silent no-op is documented, per the contract's
  "document as a silent no-op").
- Happy path byte-for-byte unchanged: the PNG writer loop and `fclose` are not
  modified; the `;;` removal changes no emitted byte (it is an empty statement).
- No new suite case (contract: E15 is probe-only).

#### Scenario: unwritable output path (E15 red→green)
- `save_as_png("/nonexistent_dir_s5/x.png")` — pre-fix: SIGSEGV (exit 139, null
  `FILE*` UB, evidence P7); post-fix: returns, exit 0, no crash, no stderr.

#### Scenario: writable output path (E15 positive control)
- `save_as_png(".work/evidence/s5_positive_control.png")` → returns `true` and the
  PNG file exists (the guard must not break the happy path).

### Acceptance (from contract)
- `grep 'if (!fp) return' matrix.hpp` ≥ 1 — matched with house spacing as
  `grep -cF 'if ( ! fp )' matrix.hpp` ≥ 1 (spacing refinement logged).
- E15 probe: exit 0 + positive control present (post-fix).
- `make test` green; PNG outputs in `examples/`/`images/` rebuild normally
  (`make example` at closeout; `git checkout -- images/` policy).

### Out of scope
`load_binary`'s adjacent unchecked `fopen` warning (S2 handoff; outside this
contract's blast radius — carried to the risk register as a watch item only);
PNG encoding/decoding; `save_as_png`'s color-map logic and return value.
