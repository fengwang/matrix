# Session 6 — Proposal

This session makes the last modernization work in the project plan real. Three
capabilities land:

1. **A real fast FFT.** `fft`/`ifft` become a true 2-D DFT: separable iterative
   radix-2 when both dimensions are powers of two (O(n² log n)), the old O(n⁴) loops
   as the documented fallback for everything else, and the missing `1/(R·C)`
   normalization on `ifft` (NumPy convention: `fft` unnormalized, round-trip
   identity).
2. **A shift that matches NumPy.** `fftshift`/`ifftshift` keep their fused
   transform+shift design but replace the swap-of-halves remap with a circular roll
   by `(n+1)/2` per axis. Even dimensions stay bit-identical; odd dimensions stop
   being wrong.
3. **A clean namespace.** The MATLAB legacy aliases (`random`, `random_like`,
   `pinverse`, `svd_inverse`, free `det(m)`) are deleted, their consumers move to
   the canonical names, and `svd_inverse`'s core becomes
   `matrix_details::pinv_core`.

Plus the verification pass (A3) and the ReadMe sweep that lands every doc delta from
sessions 2 through 5 plus this session's FFT and alias content.

## What changed the plan: the pre-flight probe

The contract assumed the pre-fix `fft` loops were a correct DFT missing only
normalization. They are not. Both loops read `x[r][c]` (the output index) inside the
inner sum instead of `x[r_][c_]` (the input index), so `fft(x)` computes
`R·C·x[0][0]` at corner (0,0) and zero elsewhere. The pre-fix baseline measured and
recorded (`.work/evidence/s6_prefix_probe.log`): `fft(8×8 δ) = 64·E₀₀` (a correct
DFT gives all ones) and `ifft(fft(x)) = (R·C)²·x[0][0]·E₀₀`. Full classification in
`failure_arbiter.md` F1 (SPEC_GAP: the contract's own executable acceptance
criteria force the corrected DFT as the reference; the descriptive C-08 note is the
part that is wrong). The oracle for differential tests is the *corrected* naive DFT
(the old loop with the data index fixed), frozen after S6 per R-18.

## Capabilities and decisions

| Capability | Decision | Source |
|---|---|---|
| `fft-core` (P1) | Approach A: single local buffer, strided in-place DIT radix-2, whole-matrix path selection; naive fallback kept | brainstorming §2, interview Q3 |
| `fftshift-roll` (C13) | Approach A: `shift ∘ transform` with a shared pure `shift_roll`; NumPy-pinned roll `(n+1)/2`; deviation documented | brainstorming §3, interview Q6 |
| `alias-retirement` (A2) | delete 5 names; `pinv` → `pinv_core`; `rand_like`/`randn_like` re-point to `rand(row, col)`; 0013 + ReadMe consumers updated | brainstorming §1 |
| `a3-verification` | pre-flight grep done (evidence in this doc's history); post-fix grep zero; note in handoff | contract A3 |
| `readme-sweep` | one new `fft` section; S2 verbatim load_npy delta; det/SVD/conv/rref/cholesky/statistics notes at existing sections; S5 NDEBUG block quoted verbatim; R-20 wide-SVD disclosure; alias-retirement table | interview Q7 |
| `fft-tests` | `tests/cases/fft.hpp` (oracle embedded, frozen) + E16/E17 `-O1` probe + E18 compile probe; pinv.hpp alias scenario dropped (F2) | brainstorming §8 |

## Open item for the user (one)

**F2 refinement (project contract §1.2: blast-radius changes are reported, never
silent):** `tests/cases/pinv.hpp` (added in S3) calls `feng::pinverse`, which A2 must
delete. The file is outside the S6 blast radius. This session adds it back into scope
for exactly one thing: dropping the alias-equivalence scenario and renaming the
TEST_CASE to "Matrix pinv". No other line changes. If you want the alias test kept,
the only alternative is keeping `pinverse` alive, which the PRD's "no backward
compatibility" rule forbids. See `failure_arbiter.md` F2.

## Not in this session (explicit)

- 2-D convolution/transpose (session 7), statistics rewrite (8), complex `rref`
  (9), parallelization/TBB (11).
- Any fix beyond the one-token data-index correction in the retained naive loops.
- `randn_like` deletion (not in the A2 list; it is re-pointed, not retired).
