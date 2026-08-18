# Project Contract — Matrix Library Upgrade

- **Status:** v1 (two-pass compiled; see `docs/prd.md` §10 for the revision record)
- **Authority chain:** `AGENTS.md` (repo expectations) → this contract (project-level law) → `docs/session_{n}_contract.yaml` (per-session authority). When in conflict, the higher document wins; conflicts are reported, not silently resolved.
- **Scope of this project:** repair + harden the single-header C++20 matrix library (`matrix.hpp`) against the 22 verified findings in `docs/opencode_sharded_review.md`, plus one bounded modernization session (real FFT + API hygiene + docs). Planning only in the blueprint turn; code is written exclusively by the development sessions.

## 1. Session lifecycle

1. A development session starts with **fresh context**, reads (in order): `AGENTS.md` → this contract → `docs/prd.md` §5–§7 → its `docs/session_{n}.md` → its `docs/session_{n}_contract.yaml` → the named finding sections of the review report.
2. Contracts are **v1 drafts**. A session may refine its own contract at start (clarify, narrow, add checks) but may not widen `blast_radius` or touch `out_of_scope` items without stopping and reporting. Refinements are logged in the session handoff decision log.
3. **Pre-flight, every session:** `git status` clean; baseline commit exists (so `git diff` vs HEAD is the authoritative change audit); re-verify each assigned finding with a minimal probe **before editing** (probe-first rule, §4). Classify any failure before fixing (BUG / SPEC_GAP / AMBIGUITY / ENVIRONMENT / TEST_BUG per `docs/prompts/failure_arbiter.md`).
4. **Post-flight, every session:** exit criteria met (§5), handoff doc written to `.work/handoff_session_{n}.md` (from `docs/templates/handoff.md`), new/changed eval seeds registered in `docs/eval_seed_cases.md`, doc deltas listed for S6.

## 2. Scope policy

- **In scope:** the 22 findings (C1–C13, S1–S2, P1–P2, A2, A3-verify, R1–R3, T1/T2 folded into fix sessions) and the S6 modernization scope. Full allocation in `docs/prd.md` §6.
- **Deferred (out of scope, named so no session "helpfully" starts them):**
  | Item | Finding | Disposition |
  |---|---|---|
  | CRTP teardown (~30 mixins → plain members) | A1 | own future project; touches every method, exceeds one 128K session |
  | Full elementwise ADL namespacing (`feng::elem::`) | A3 (full) | risk register R-08; verified this turn: the finding as described is unsupported in the current code (no `feng::elem::`, no `elem` namespace) — S6 does a verification pass + policy note only (no code change expected) |
  | DLPack / NumPy interop | — (research-doc topic) | parked; no consumer exists in this repo |
  | openimageio image I/O, `cuda_matrix`, `concatenate` | — (REVIEW.md TODOs) | future feature work, not findings |
- **No new features.** Anything not traceable to a finding row or the S6 scope is out of scope.

## 3. API policy

- **Canonical-name rule:** the public name of an operation is the name the `ReadMe.md` documents. Undocumented duplicate names are retired (deleted) in S6. Current canonicals: `rand`/`rand_like` (not `random`/`random_like`), `pinv` (not `pinverse`/`svd_inverse`), member `m.det()` (not free `det(m)`).
- **Behavior changes:** the only sanctioned public behavior changes are the rows of `docs/prd.md` §5 (the table). This table *is* the authorization `AGENTS.md` requires ("do not change public API behavior unless the contract says so"). Any required change outside the table stops the session.
- **Signatures:** no signature change except where a table row says so (e.g., `cholesky_decomposition` `void → bool` in S4). `noexcept` may be dropped where the body can throw (allocation); that is not a source-breaking change and needs no table row.

## 4. Verification rules

- **Primary check:** `make test` — full Catch2 suite green, every run, before any claim of done.
- **Regression tests ship with fixes (T1/T2 policy):** every correctness/security fix lands with at least one new `tests/cases/*.hpp` case asserting *content*, registered in `tests/test.cc`. A test that would still pass with the bug present is not a regression test.
- **ASan probes** (mandatory for S1, S2; optional elsewhere): `g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1 -o .work/probe probe.cc` — `NDEBUG` on purpose, so `better_assert` is silent and *real* OOB behavior is observable (method of the review report).
- **Deterministic evidence only:** every done-claim cites command output or code evidence. "It looks right" is not evidence.
- **Probe-first rule:** a finding is re-verified with a minimal probe before its fix is written. Mandatory for derived findings — currently C13 (the bug's existence was code-verified this turn; the *fix target* — NumPy's pinned roll — is still probe-validated even+odd at S6 pre-flight) — and for any finding whose review evidence conflicts with a reference (see `docs/evidence_map.md` §conflicts).
- **Failure classification:** before fixing a failing check, classify per `docs/prompts/failure_arbiter.md`; the handoff records category + evidence.
- **Final answer of every session states checks run and checks not run** (AGENTS.md).

## 5. Exit criteria (all sessions)

1. `make test` green (and `make example` for S3/S6, which change `examples/`-visible behavior).
2. All `acceptance_criteria` of the session contract pass with cited output.
3. `git diff --name-only HEAD` ⊆ `blast_radius.allowed_files` (plus the handoff/eval-seed doc updates).
4. Sharded review along the 6 axes (`docs/prompts/sharded_review.md`) with findings dispositioned.
5. Adversarial verifier (`docs/prompts/adversarial_verifier.md`) returns PASS.
6. High-risk sessions (S1, S2) additionally: **human decision gate** — the user reviews diff + evidence before merge.
7. Handoff doc complete: state snapshot, decision log (incl. contract refinements), doc deltas, eval seeds, warnings.

## 6. Orchestration rules (from AGENTS.md, binding here)

- One subagent unit ≤ 3 items, ~≤35 tool calls; phrase units as **end states** ("ensure X holds; check first, edit only if not").
- Read-only agents return findings inline, verdict on the FIRST line; write-capable agents put full output in `.work/`, return ≤10 lines. Never hand output via `/tmp` — use `.work/` or inline.
- Commit before any multi-agent edit wave; `git diff` vs HEAD is the cheap authoritative check.
- Shared decisions (canonical path, artifact owner, naming) are resolved by the orchestrator **before** dispatch and stated identically in every prompt.

## 7. Code anchors

- **Function names are authoritative; line numbers are hints.** The review report's line numbers were captured 2026-07-13; the current file re-verified as the same revision (7,688 lines vs 7,689). Re-verification this turn found **three** review structural claims that do not hold (R2's range; A3's `norm` lines / `feng::elem::` call; C13's remap description) and **two extra** `load_npy` hazards the review missed (header_length overflow; unguarded `npos` shape parse) — all recorded in `docs/evidence_map.md` conflicts C-08–C-11. Current verified anchors:
  | Symbol | Line (current) |
  |---|---|
  | `better_assert` macro | 84–95 |
  | `parallel` (guarded) / `reduce` (unguarded) / 2nd unguarded site | 276 / 1152 / 4036 |
  | `crtp_load_npy` (`load_npy` members) | 2499 (2504, 2508) |
  | `save_png` (free helper) / call site | 3096 / 3384 |
  | `crtp_shrink_to_size` / buggy copy | 3508 / 3531–3532 |
  | `matrix` class + CRTP base list | 3723 (3760) |
  | `flipdim` / buggy `swap_ranges` / `fliplr` / `flipud` | 4453 / 4479 / 4491 / 4496 |
  | free `det(m)` (A2 duplicate) | 4319–4321 |
  | `crtp_det` (Schur, `P.inverse()`) | 2048 |
  | `cholesky_decomposition` | 5676 |
  | `svd_inverse` / `pinverse` / `pinv` / `rand` / `rand_like` / `random` / `random_like` | 5216 / 5226 / 5233 / 5240 / 5272 / 5262,5267 / 5278 |
  | `operator^` (buggy odd branch) | 5555 (5566) |
  | `forward_substitution` / `gauss_jordan_elimination` / `rref` | 6368 / 6393 / 6427 |
  | `fft` / `fftshift` / `ifft` / `ifftshift` | 6313 / 6349 / 6446 / 6480 |
  | `conv` (full / mode) | 6573 / 6611 (asserts ~6620) |
  | `lu_decomposition` (int / tuple) | 6499 / 6532 |
  | unary functions block / binary functions block | 6840–7189 / 7210–7535 |
  | `mean` / `variance` / `standard_deviation` | 7640 / 7646 / 7652 |
- Read a **named region** (function + ~20 lines context), never the whole header.

## 8. Environment

- Archlinux, zsh; gcc/llvm, gdb, valgrind, strace/ltrace, perf, systemtap installed; docker available if host is insufficient.
- Build: `make test` (Catch2 suite), `make example` (examples); C++20, `-DPARALLEL` per Makefile. Review used GCC 16.2; any GCC with full C++20 support is acceptable — record the compiler version in each handoff.

## 9. Document map (this project)

| Document | Role |
|---|---|
| `docs/prd.md` | Goal, intent, sanctioned behavior changes, session map, policies |
| `docs/evidence_map.md` | Claim → evidence → source → confidence → gap, for every major recommendation |
| `docs/risk_register.md` | Live risks, mitigations, owners |
| `docs/eval_seed_cases.md` | Standing deterministic probes (E01–E18), fast smoke set for the eval loop |
| `docs/session_{n}.md` | Human story outline for session n (objective, scope, deliveries, exit criteria) |
| `docs/session_{n}_contract.yaml` | Machine-readable session authority (schema fixed by the project) |
| `docs/opencode_sharded_review.md` | Source findings (2026-07-13); evidence base, read per-finding |
| `docs/deep_research/*.md` | Research context & semantics authority (see PRD §9) — **never read in full inside a session** |
| `docs/prompts/*` | Review/verifier/arbiter/harvest prompt definitions (workflow) |
