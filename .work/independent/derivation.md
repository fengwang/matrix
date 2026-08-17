# Session 1 — Independent test-writer derivation (branch_and_compare)

Fresh-context subagent (Qwen3.8-27B, runId wf_msxq6npb-4-94d3a0fc21f7, 13s). Inputs were
pasted-only: the two documented contract rules + API signatures + case list. The agent did NOT
read matrix.hpp, the worker's tests, or the implementation diff.

Agent output (verbatim):

    i1: 5x3 all 1.0
    i2: [[1,2],[11,12],[21,22],[0,0],[0,0]]
    i3: 4x4 with 7.0 at [0][0], all other 15 elements 0
    i4: [[5,4,3,2,1],[10,9,8,7,6],[15,14,13,12,11]]
    i5: [[4,3,2,1],[8,7,6,5],[12,11,10,9],[16,15,14,13]]
    i6: [[11,12,13,14,15],[6,7,8,9,10],[1,2,3,4,5]]

Contract rules given to the agent (its only authority):
- shrink_to_size(new_row,new_col): result shape (new_row,new_col); top-left
  min(old_row,new_row) x min(old_col,new_col) block keeps original values exactly; all other
  result elements zero. (In-code documented comment, matrix.hpp ~3515-3517; PRD 5 row 1.)
- flipdim(m,2): f[r][c] = m[r][col-1-c]; flipdim(m,1): f[r][c] = m[row-1-r][c]; shape unchanged.
  (PRD 5 row 2; review C2 violated-contract clause; MATLAB/NumPy convention.)

Comparison vs worker's expectations (specs/session_1/specs/*.md, written pre-implementation
from the same contracts): **i1..i6 all AGREE.**
