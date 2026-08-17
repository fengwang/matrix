# Session 2 — Tasks

Ordered by dependency. Each task is verifiable (done = its check passes; checks defined in
`plan.md`). Specs: `specs/`; approach: `design.md`; contract: `docs/session_2_contract.yaml`.

## 1. Pre-flight (evidence base — done before any edit)

- [x] 1.1 Baseline: `git status` inspected on `phase-1/session-2` @ `ad6fa79`; `make test` +
      `./test_test` green (59 cases, 49,216,776 assertions) — `.work/evidence/baseline_*` (S1's
      logs) + re-run this turn (session-start baseline).
- [x] 1.2 Re-anchor by grep: `crtp_load_npy` at `matrix.hpp:2499` (members 2504/2508); review
      structure confirmed (fixed-offset derefs, unguarded `stoul`, no dtype check). The two
      C-11 hazards verified this turn (header_length overflow; `npos` shape parse). **Third
      hazard found + confirmed empirically**: shape/payload arithmetic overflow-unguarded —
      `(-1, 2)` → `stoul` → `resize` throws `bad_array_new_length` → terminate.
- [x] 1.3 Re-anchor `load_bmp` model by grep: now at `matrix.hpp:6643` (review's ~6760 stale —
      R-02 logged); pattern read (validate size/consistency before parsing; early failure return).
- [x] 1.4 Pre-fix ASan reproduction (probe-first, P5): `.work/probes/E03_E04.cc` built with the
      contract flags; 18 individual case runs recorded in `.work/evidence/prefix_*` — 4 ASan
      OOB reads, 4 terminate paths, 4 silent misloads (`ok=1`), 4 passing pins. All review
      claims reproduced → proceed.
- [x] 1.5 Fixture headers hex-dumped (≤64B each): magic/version/descr/shape offsets confirmed
      from bytes; exact descr strings `|u1` / `|i1` / `<f4` / `<f8`.
- [x] 1.6 Phase docs written (brainstorming, proposal, design, 3 specs, tasks, plan, execution
      contract); pre-flight checkpoint committed.

## 2. Negative tests (TDD red)

- [ ] 2.1 Append exactly 5 negative `TEST_CASE`s to `tests/cases/load_npy.hpp` per spec
      R-R1/R-V scenarios (truncated magic 3B + missing file; truncated header 11B + 21B;
      malformed/missing shape token incl. `(-1, 2)`; `header_length` 0xFFFFFFFF; float32-into-
      double + `>f8`). Crafted bytes → `tmp/`, removed per case; each case also asserts matrix
      state unchanged.
- [ ] 2.2 TDD red evidence: `make test` + run `./test_test "[load_npy]"` on the pre-fix tree →
      the 4 happy cases pass, the 5 new cases fail/crash (record output); existing blocks
      byte-unchanged (`git diff` on the test file = append only).

## 3. Fix: `crtp_load_npy` validated boundary

- [ ] 3.1 Implement spec R-V1…R-V9 in the `load_npy(char const*)` body (single edit; body only):
      magic/size/version, non-wrapping header bound, dict sanity, dtype map, digit-bounded shape
      parser, overflow-checked payload bound, resize-after-validation, `int8_t*` byte copy,
      `try/catch(…) → false`.
- [ ] 3.2 Targeted green: `make test` + `./test_test "[load_npy]"` → 9/9 cases pass; full
      `make test` + `./test_test` → 64 cases green.
- [ ] 3.3 ASan probe green: rebuild `.work/probe_s2` (contract flags); full run → `PASS E03` +
      `PASS E04`, exit 0, no ASan report.

## 4. Full checks + audit

- [ ] 4.1 Full suite log (`.work/evidence/final_suite_run.log`); compiler version recorded.
- [ ] 4.2 Diff audit vs `ad6fa79`: `git diff --name-only` ⊆ allowed set; `matrix.hpp` diff =
      exactly the `load_npy` body region (no other hunk); test file diff = append only.
- [ ] 4.3 Contract deterministic check `grep -c 'return false'` on `matrix.hpp` lines 2499–2590
      > 6.
- [ ] 4.4 Happy-path invariance: `git diff` of `tests/cases/load_npy.hpp` shows the 4 existing
      blocks untouched.

## 5. Independent derivation + bug-restoration (branch_and_compare)

- [ ] 5.1 Independent test-writer derivation (fresh framing, contract-only inputs — no diff
      read): expected accept/reject per P3 checklist re-derived; concurred with the suite's
      expectations (recorded in `.work/independent/`).
- [ ] 5.2 Bug-restoration check: temporarily restore a pre-fix hazard (e.g. drop the
      `header_length` bound) → the 0xFFFFFFFF case must FAIL/crash; restore the fix → green.
      Never commit the temp state.

## 6. Sharded review (6 axes)

- [ ] 6.1 Run correctness / readability / security / tests / architecture / performance axes
      over the diff (per `docs/prompts/sharded_review.md`); findings to
      `docs/session_2/sharded_review.md`.
- [ ] 6.2 Fix High/Critical findings only; re-run task 4 checks after each fix.

## 7. Adversarial verification

- [ ] 7.1 Fresh-context verifier (per `docs/prompts/adversarial_verifier.md`) sees contract +
      diff + evidence only; focus: attacker-chosen bytes (3B/11B/12B, 0xFFFFFFFF, exact
      boundaries, missing shape, big-endian dtype, real-spec v2); verdict to
      `docs/session_2/adversarial_verification.md`. FAIL → failure arbiter first.

## 8. Closeout

- [ ] 8.1 Eval seeds E03/E04 → `promoted` in `docs/eval_seed_cases.md` (probe + permanent home).
- [ ] 8.2 `docs/risk_register.md`: S2 closeout watch items (3rd hazard; adjacent `load_binary`
      overflow-unguarded arithmetic — doc only).
- [ ] 8.3 Handoff `.work/handoff_session_2.md` (template): snapshot incl. compiler, checks
      run/not run, decision log (pre-fix evidence, 3rd hazard, v2 convention, zero-dim
      interpretation, `docs/handoff.md` vs `.work/` path decision), S6 doc deltas (exact ReadMe
      wording), S5 warning (`save_png`).
- [ ] 8.4 Re-run final checks; done-condition verification; final commit; present diff +
      evidence for the human decision gate.
