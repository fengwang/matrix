# Failure Arbiter — Session 6

Per `docs/prompts/failure_arbiter.md`. Entries are recorded **before** any fix is attempted.

## F1 — Pre-flight: the pre-fix `fft`/`ifft` are not DFTs at all (disproves PRD note C-08)

- **Failing check:** the pre-flight probe expectation derived from PRD revision-record note C-08
  and the S6 contract invariants ("fft results identical (within 1e-9 double) to the pre-change
  naive DFT for all tested shapes (**the old code is the oracle**)"; "P1 baseline: record the
  current `ifft(fft(x)) == R·C·x` behavior (pre-normalization) so E16 shows the change").
- **Evidence:** `.work/evidence/s6_prefix_probe.log` (probe `.work/probes/S6_prefix_c13_p1.cc`,
  built `-O1` at HEAD d5e7b56):
  - `fft` of the 8×8 delta at (0,0) returns `64·E₀₀` (max |X − 64·E₀₀| = 0 to ~1e-14); a correct
    DFT returns all ones.
  - `ifft(fft(x))` for x = 3·ones₈₈ + δ₀₀ returns `4096·x[0][0]·E₀₀` (max deviation 2.5e-27) —
    i.e. `(R·C)²·x[0][0]` at (0,0), zeros elsewhere. Not `R·C·x`, not `x`, not a scaled identity.
  - Source cause (matrix.hpp, `fft` ~6426 and `ifft` ~6557): both inner sums read
    **`x[r][c]` (the output indices) instead of `x[r_][c_]` (the input indices)**:
    `tmp += x[r][c] * make_omege( c, c_, C )`. Algebraically the kernel sums
    `Σ_{c_} e^{−2πi·c·c_/C}` vanish for `c ≠ 0`, so
    `X[r][c] = R·C·x[0][0]·δ_{r,0}·δ_{c,0}`. The loops are O(n⁴) but compute a rank-1 corner,
    not a DFT.
- **Category:** **SPEC_GAP.** The contract/PRD misdescribes the pre-fix code (C-08's "the loops
  are a *correct* 2-D DFT" is unsupported — the planning turn's full read missed the data-index
  bug), and the contract is internally inconsistent: the invariants "the old code is the
  oracle" + "baseline R·C·x" contradict its own executable acceptance criteria
  ("fft of 8×8 delta ⇒ all ones within 1e-9", "‖ifft(fft(x)) − x‖∞ < 1e-9 (8×8)").
- **Why other categories do not fit:**
  - Not BUG: no S6 implementation exists yet; what fails is the contract's description of
    reality, not code we wrote.
  - Not AMBIGUITY: the desired end state is unambiguous (the acceptance criteria are
    executable and force a true DFT with identity round-trip). Only the *oracle's provenance*
    is misdescribed, and exactly one self-consistent reading exists.
  - Not ENVIRONMENT / TEST_BUG: the probe is deterministic and correct; nothing flaky.
- **Allowed next action:** stop; adopt the explicit interpretation below; log the contract
  refinement (policy P9: narrow/clarify allowed at session start); proceed.
- **Forbidden next action:** pinning the broken pre-fix behavior as the differential oracle;
  weakening E16 or the invariants to match the broken behavior; shipping a `fft` that is
  correct on power-of-2 shapes but garbage on non-power-of-2 shapes.

### Chosen interpretation (binding for S6, logged per P9)

1. **Reference semantics:** a mathematically correct 2-D DFT under the NumPy `fft2` convention:
   `fft` unnormalized, `ifft` with the single `1/(R·C)` normalization (identity round-trip).
   This is the sanctioned end state (PRD §5 row 16) and is unchanged by this finding.
2. **Oracle / fallback provenance:** the "retained naive implementation" (PRD §5 row 16, policy
   P6, R-18) is the **correct** naive 2-D DFT — the existing loop structure with the data index
   corrected to `x[r_][c_]` (one-token fix, both `fft`/`ifft` sides; kernel, signs, and
   `add_complex` promotion untouched). The embedded oracle in `tests/cases/fft.hpp` is a copy of
   that corrected naive DFT, frozen after S6 (R-18 principle preserved: the oracle is a
   head-baseline copy — of the *correct* baseline behavior, which the pre-fix code did not
   compute).
3. **Pre-fix baseline (recorded, supersedes the "R·C·x" note):** `fft(x) = R·C·x[0][0]·E₀₀` and
   `ifft(fft(x)) = (R·C)²·x[0][0]·E₀₀` up to ~1e-14 kernel-sum residue (measured,
   `.work/evidence/s6_prefix_probe.log`). E16's "pre-fix baseline recorded first" requirement is
   satisfied by this log (the change shown is larger than the PRD anticipated).
4. **C13 note:** the pre-fix `fftshift`/`ifftshift` outputs are degenerate as *value*
   comparisons because of the fused design (shift∘broken-transform) — only the position of the
   single nonzero corner entry is observable (n=4 → row 2, n=5 → row 3). The C13 remap itself
   was verified by a faithful transcription simulation of the pre-fix swap block:
   n=4 `(2,3,0,1)` == NumPy roll; n=5 `(3,4,2,0,1)` ≠ NumPy `(2,3,4,0,1)` (the documented bug).
   Post-fix, the real E17 probe compares full values against a true-DFT NumPy-roll reference.

## F2 — Pre-audit: `tests/cases/pinv.hpp` (S3) calls the name A2 must delete

- **Failing check (anticipated):** the A2 acceptance criterion
  `grep -c 'pinverse\|svd_inverse' matrix.hpp == 0` forces deleting `pinverse`, but
  `tests/cases/pinv.hpp:53` (added in S3, file outside the S6 `blast_radius`) calls
  `feng::pinverse( m )` in the scenario "pinv == pinverse (same core, bitwise)". After the
  deletion, `make test` would not compile.
- **Evidence:** `grep -n pinverse tests/cases/pinv.hpp` (line 53, plus the TEST_CASE name
  "Matrix pinv/pinverse" at line 2 and header comments lines 4–6 mentioning
  `pinverse`/`svd_inverse`); the S6 contract
  `blast_radius.allowed_files` does not list `tests/cases/pinv.hpp`; the A2 consumer audit
  (evidence map row A2) predates S3 and states "no in-repo use of `pinverse`" — that audit is
  now stale by exactly this one call. Verified pre-flight: `grep -c 'pinverse\|svd_inverse'
  matrix.hpp` = 4 (lines 5307, 5317, 5319, 5324 — the last is `pinv`'s own body calling
  `pinverse`, also re-pointed in A2); `pinv`/`pinverse`/`svd_inverse` currently share one
  hard-coded threshold `|w| > 1.0e-10` (no tolerance parameter exists in the code, despite
  the S3 handoff's "tolerance=0.01" phrasing).
- **Category:** **SPEC_GAP** (the blast radius does not account for a post-audit test that pins
  the retired alias), with a **TEST_BUG** component (the scenario asserts a feature — alias
  existence — that the contract mandates be removed; a test that would fail by contract is not
  a regression test of the canonical behavior).
- **Why other categories do not fit:**
  - Not BUG: no S6 code written yet; the conflict is between the A2 deletion mandate and the
    S3 test file's alias pin.
  - Not AMBIGUITY: both mandates are explicit; they simply intersect.
- **Allowed next action (requires the logged + reported refinement, project contract §1.2):**
  add **one** file to the S6 blast radius — `tests/cases/pinv.hpp` — for a **canonical-name
  update only**: delete the "pinv == pinverse" alias-equivalence scenario, rename the TEST_CASE
  to "Matrix pinv", and fix the header comments to reference the canonical `pinv` /
  `matrix_details::pinv_core`. Nothing else in the file changes. **Reported** in the
  interview doc, execution contract, handoff decision log, and the session's final answer
  (§1.2: blast-radius expansion is never silent).
- **Forbidden next action:** keeping `pinverse` in `matrix.hpp` to satisfy the stale test
  (violates the acceptance grep + PRD §5 row 15); rewriting more of `pinv.hpp` than the
  alias scenario/naming; silently editing the file without logging the refinement.

## F3 — Pre-audit: the A3 grep gate and the A2 include-preservation invariant collide

- **Failing check (anticipated):** A3 acceptance `grep -c 'random\b' matrix.hpp == 0` vs
  the A2 invariant "the `#include <random>` line (S5 dependency) is **kept**" (and the
  verified fact that `#include <random>` at matrix.hpp:29 **matches** `random\b` — the
  `>` after the identifier is a word boundary; measured pre-fix: `grep -c 'random\b'
  matrix.hpp` = 4 = the include + the two overloads + the `rand_like` call).
- **Category:** **AMBIGUITY** (the acceptance criterion's intent — zero *identifier* uses
  of the retired aliases — is clear, but its literal count is unsatisfiable while the
  same contract's invariant mandates the include; `std::mt19937` in S5's `rand` requires
  the include, so deleting it is not a permitted resolution).
- **Resolution (forced by the document set):** the gate is interpreted over identifier
  use. Post-fix, `grep -n 'random\b' matrix.hpp` must return **exactly one line — line 29,
  `#include <random>`** — and nothing else; the match list (not just the count) is recorded
  in the evidence log. `std::random_access_iterator_tag` (line 151) does not match
  (`random_` has no boundary after `random`). No other resolution exists: keeping the
  include is invariant-mandated, and the PRD's no-backward-compatibility rule forbids
  re-adding any alias to make a count work.
- **Allowed next action:** record the post-fix match list in
  `.work/evidence/s6_final.log` and the handoff; state the interpretation in the A3 spec.
- **Forbidden next action:** deleting `#include <random>` to satisfy the literal count
  (breaks the A2 invariant and the S5 build); treating a post-fix count of 1 (the include
  line only) as a pass without listing and verifying that line.
