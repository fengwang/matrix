# Session 6 — Interview (intent extraction)

Status: complete. Per the interview-me method, the session contract set was stress-tested
question by question until every residual decision point was resolved from the dominant
documents or explicitly escalated. **One point requires live user visibility (Q1, a
blast-radius refinement mandated by project contract §1.2 "stop and report"); all other
points are forced by the contract's own executable acceptance criteria** (same pattern as
S4: executable acceptance criteria outrank conflicting descriptive lines).

HYPOTHESIS: the user wants the S6 contract executed end to end — true fast `fft`/`ifft` with
the `1/(R·C)` normalization, the NumPy-pinned `fftshift`/`ifftshift` roll, A2 retirements,
the A3 verification pass, and the full ReadMe sweep — with TDD, content-asserting tests,
deterministic probes E16/E17/E18, sharded review, adversarial verification, and the
project-closing handoff — with zero scope creep beyond `docs/session_6_contract.yaml`.
CONFIDENCE at session start: ~90% (two pre-flight findings: F1 pre-fix `fft`/`ifft` are not
DFTs at all; F2 an S3 test pins a name A2 must delete); **~96%** after the resolutions below.

## Stress-test questions

### Q1 — `tests/cases/pinv.hpp` calls `feng::pinverse`, which A2 must delete (escalated; F2)

The A2 acceptance criterion (`grep -c 'pinverse\|svd_inverse' matrix.hpp == 0`) forces
deleting `pinverse`, but `tests/cases/pinv.hpp:53` (added in S3, **outside the S6 blast
radius**) pins the alias in the scenario "pinv == pinverse (same core, bitwise)". The two
mandates intersect: delete the name → suite does not compile; keep the name → the grep and
PRD §5 row 15 fail. The A2 consumer audit (evidence map) predates S3 and is stale by exactly
this one call.

**Resolution (requires the user's awareness — project contract §1.2: blast-radius expansion
is never silent):** add **one** file to the S6 blast radius — `tests/cases/pinv.hpp` — for a
**canonical-name update only**: delete the alias-equivalence scenario (it asserts a feature
the contract mandates be removed; a test that fails by contract is a TEST_BUG), rename the
TEST_CASE to "Matrix pinv", fix the header comments. No other line of that file changes.
Logged in `failure_arbiter.md` F2, the execution contract, and the handoff decision log;
reported in the session's final answer.
**Rejected alternative:** keep `pinverse` as a deprecated shim (violates the user's explicit
"no backward compatibility / remove deprecated code" constraint and the acceptance grep).

### Q2 — The pre-fix `fft`/`ifft` loops are not a correct DFT (resolved from documents; F1)

PRD note C-08 (and the S6 invariants "the old code is the oracle", "pre-fix baseline
`ifft(fft(x)) == R·C·x`") describe the pre-fix loops as a *correct* O(n⁴) DFT missing only
normalization. The pre-flight probe (`.work/evidence/s6_prefix_probe.log`) disproves it: both
loops read `x[r][c]` (output index) inside the inner sum instead of `x[r_][c_]` (input), so
they compute `R·C·x[0][0]·E₀₀` — a rank-1 corner, not a DFT.

**Resolution (forced by the contract's own executable acceptance criteria):** the reference
is the mathematically correct 2-D DFT (NumPy `fft2` convention: `fft` unnormalized, `ifft`
× `1/(R·C)`). The retained naive fallback and the embedded differential oracle are the
**corrected** naive DFT (existing loop structure, data index fixed to `x[r_][c_]`; kernel,
signs, and `add_complex` promotion untouched). Pre-fix baseline recorded as measured
(`fft(x) = R·C·x[0][0]·E₀₀`, `ifft∘fft = (R·C)²·x[0][0]·E₀₀`), which supersedes the
"R·C·x" note. The sanctioned end state (PRD §5 row 16) is unchanged. Classification and
full reasoning: `failure_arbiter.md` F1 (SPEC_GAP — the contract misdescribes pre-fix
reality; the acceptance criteria are unambiguous and dominate the descriptive note).
**Rejected alternative:** keep the broken loop as the literal "retained fallback"
(constructionally possible, but ships a `fft` that is correct on power-of-2 shapes and
garbage on others — contradicts PRD goal 3 "honest performance" and would make the 6×8
differential test pin broken behavior).

### Q3 — FFT fast-path selection: whole-matrix vs per-axis (resolved from documents)

The contract admits two readings of P6 ("power-of-2 sizes only for the fast path; all other
sizes use the retained naive implementation"): (a) **whole-matrix** — fast only when *both*
dimensions are power-of-2, else the full naive fallback; (b) **per-axis hybrid** — fast
radix-2 on power-of-2 axes, naive 1-D on the others (so 126×128 gets a fast column axis).

**Resolution:** whole-matrix selection (a). Rationale: P6's wording is "the retained naive
**implementation**" (the existing 2-D loops moved to a private helper), not "a 1-D naive
fallback per axis"; the exit criterion "fallback path selected for 6×8" and the
adversarial case "126×128 (one dim fallback)" are both satisfied by whole-matrix selection;
(a) is strictly less new code and less new risk surface (no mixed-stride path to get wrong),
and the naive fallback remains the documented safety net exactly as the PRD frames it
("no new math, the old code is the safety net" — the corrected old 2-D code).

### Q4 — `randn_like` after the `random_like` deletion (resolved from documents)

`randn_like` (matrix.hpp ~5374) is **not** in the A2 retirement list, but its body calls
`rand_like` (which A2 deletes) — a dangling reference after the change (adversarial case:
"A2 leaving no dangling references"). Zero in-repo consumers of `randn_like`
(grep over tests/, examples/, ReadMe.md: none).

**Resolution:** keep the `randn_like` name (not sanctioned for deletion — the A2 list is
exhaustive), re-point its body to `rand< T, A >( row, col )` (behavior-identical: the old
chain `randn_like → rand_like → random → rand(seed 0)` and the new one-arg form
`rand< T, A >( n ) → rand( n, n, seed 0 )` both yield the seed-0/time-based stream).
Documented in the decision log. **Rejected alternative:** also delete `randn_like`
(scope creep — the A2 list and PRD §5 row 15 are exhaustive; a new retirement row is a new
sanctioned decision).

### Q5 — `ifft` normalization placement and application count (resolved from documents)

The contract forbids the classic slip: "normalization applied to `fft` by mistake (it goes
on `ifft` only)" and "applied exactly once". **Resolution:** the factor `1/(R·C)` is
applied in `ifft` only, as a single post-transform scaling of the result (fast and naive
paths alike); `fft` is unnormalized (NumPy convention). Pinned by E16 round-trip
(`‖ifft(fft(x)) − x‖∞ < 1e-9` fails if the factor is missing, doubled, or on `fft`) and by
the adversarial double-application probe (`ifft(ifft(x))` scaling = `1/(R·C)²`).

### Q6 — `fftshift`/`ifftshift` semantics for odd n (resolved from documents; probe-first done)

The contract pins both functions to the circular roll by `(n+1)/2` per axis (E17: n=3 →
both `(1,2,0)`; n=4 → both `(2,3,0,1)`; C13 note: n=5 → `(2,3,4,0,1)`), and keeps the fused
transform+shift design (deliberate deviation from NumPy's pure reindex; NumPy's own
`ifftshift` differs from `fftshift` for odd n — the library's fused design applies the same
roll to both). **Resolution:** both public functions apply the identical per-axis roll of
`(n+1)/2`; even-n behavior is bit-identical to pre-fix (swap-of-halves == roll by n/2,
verified by the pre-flight transcription probe). The deviation is documented in the ReadMe
FFT section and the handoff.

### Q7 — Where the ReadMe additions land (resolved from code/ReadMe reading)

The ReadMe has no FFT section (the trailing `fft/fftshift/...` lines ~2382–2385 sit inside a
commented-out MATLAB-function wishlist; the API-reference block has a `pinv` line but no
`rand` line and no `random`/`pinverse`/`svd_inverse` lines). **Resolution:** one new prose
section "#### fft -- fast Fourier transform" placed after the `lu decomposition` section
(~line 1800, with the other transform/linear-algebra prose), containing: the radix-2 +
fallback rule + complexity, the `ifft` normalization, the `fftshift`/`ifftshift`
convention + fused-design note, and the alias-retirement table; the S2 `load_npy` delta
verbatim after its code block (~1063); the det/lu/conv/rref/cholesky/statistics notes
inserted at their existing sections; the S5 NDEBUG policy block as a short
"#### assertions and `NDEBUG`" section near the build/usage area; the R-20 wide-SVD
disclosure at the `pinv` API line; the `random` → `rand` example fix at ~1277.

### Q8 — Test placement and suite-safety (resolved from code reading)

Suite build is `-Ofast` (fast-math, R-19): the new `tests/cases/fft.hpp` assertions use
tolerances on finite values only (no NaN-dependent checks; all FFT pins use fixed finite
inputs); exact behavior pins live in the `-O1` probe (`.work/probes/E16_E17.cc`).
Placement: `tests/cases/fft.hpp` registered in `tests/test.cc` at the verified alphabetical
position (`fabs < fft < flip`). The oracle in `fft.hpp` is a self-contained copy of the
corrected naive DFT (frozen per R-18; header comment records the provenance and the F1
finding).

**Interview verdict: ready to brainstorm with one escalated item (Q1) flagged for the user;
all other points resolved.**
