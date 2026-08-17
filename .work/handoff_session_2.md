# Session Handoff

## State Snapshot

- **Session:** 2 — `load_npy` validated input boundary (finding S1; T2 error-path gap)
- **Branch:** `phase-1/session-2`
- **Last commit:** `88c3740` (fix + audit + bug-restoration evidence); closeout commit follows
  (sharded review, adversarial verification, seeds, risk register, this handoff).
  Commit chain off baseline `ad6fa79` (S1 closeout): `0cbef65` (pre-flight docs/probe/evidence)
  → tests-red commit → `88c3740`.
- **Changed files (vs `ad6fa79`):** `matrix.hpp` (single hunk, `crtp_load_npy::load_npy( char
  const* )` body only), `tests/cases/load_npy.hpp` (append-only: 5 negative `TEST_CASE`s +
  file-local helpers; existing `TEST_CASE( "Loading npy files" )` byte-identical),
  `docs/session_2/**` (phase docs + `sharded_review.md` + `adversarial_verification.md`),
  `docs/eval_seed_cases.md` (E03/E04 → promoted), `docs/risk_register.md` (S2 watch items),
  `.work/**` (probe, evidence logs, independent derivation, this handoff). Nothing else —
  verified by `git diff --name-only ad6fa79` ⊆ contract allowed set.
- **Checks run:**
  - Baseline (pre-edit): `make test` + full suite green (59 cases / 49,216,776 assertions) +
    18-case pre-fix ASan probe reproduction (4 ASan OOB reads, 4 `terminate` paths, 4 silent
    misloads, 4 pins) — `.work/evidence/prefix_*`.
  - TDD red (pre-fix): 5 new cases — SIGABRT ×2 (assert/terminate), SIGSEGV (0xFFFFFFFF),
    false-positive `true` ×2 (silent misload) — `.work/evidence/tdd_red_*.log`.
  - `make test` + full suite green: **64 cases / 49,216,811 assertions, exit 0**
    (`.work/evidence/final_suite_run.log`); `./test_test "[load_npy]"` 6/6.
  - ASan probe (contract flags `-std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1`):
    `.work/probe_s2` → `PASS E03` + `PASS E04`, exit 0 (`.work/evidence/probe_green_final.log`).
  - Extra adversarial attacks (10 inputs beyond the probe, incl. wrap-product, 1 MB header,
    directory-as-name, exact boundaries): `PASS EXTRA-ATTACKS`, exit 0
    (`.work/evidence/probe_adv_attacks.log`).
  - Bug-restoration: `header_length` bound disabled → 0xFFFFFFFF case segfaults (exit 139);
    restored → green (`.work/evidence/bug_restore_red.log`).
  - Contract deterministic check: `sed -n '2499,2590p' matrix.hpp | grep -c 'return false'`
    = **12** > 6.
  - Diff audit: single hunk `@@ -2509 +2509` inside the function; test diff 0 deleted lines.
  - Sharded review (6 axes, `docs/session_2/sharded_review.md`): 0 Critical/High, 2 Low
    (fixed: bad-magic pin, zero-dim pin), 3 Info.
  - Adversarial verification (`docs/session_2/adversarial_verification.md`): **PASS**.
  - Compiler: `g++ (GCC) 16.2.1 20260810`.
- **Checks not run:** `make example` (no example/`main.cpp` touched; no consumer of `load_npy`
  in `examples/` — the change is rejection-only and examples load valid fixtures); ASan on the
  *full* suite (the suite build is the assert-enabled `-Ofast` build; the ASan coverage is the
  dedicated probe binaries, per the contract's evidence list); no Valgrind/fuzzing/clang or
  Windows cross-check (host is Linux/g++ only); no performance benchmark (no hot-path change —
  single buffer read, one byte-copy, linear header parse; pre-fix did the same IO).
- **Current status:** done condition met; **awaiting human decision gate** (high-risk session:
  diff + evidence presented below; no merge before sign-off).

## Narrative Context

`load_npy` is the library's only binary matrix import and treated file bytes as trusted: the
pre-fix probes reproduced ASan out-of-bounds reads on 3B/11B/`0xFFFFFFFF` files, four distinct
`std::terminate` paths (unguarded `stoul` throwing out of the `noexcept` member, including a
newly found third hazard — overflow-unguarded shape/payload arithmetic, `stoul("-1")` →
`resize` → `bad_array_new_length`), and silent misloads of foreign-dtype and short-payload
files. The function body is now a validate-then-act boundary (the in-repo `load_bmp`/
`load_binary` models): magic/size/version before any deref, non-wrapping `header_length` bound,
dict-literal sanity, dtype exact-match against `value_type`, npos-guarded digit-bounded shape
parse with non-zero dims, overflow-checked payload bound, `resize` strictly after validation,
and a `try/catch(…)` so the `noexcept` member is genuinely throw-free. Five content- and
state-asserting negative test cases (TDD: red pre-fix, green post-fix) close the T2 gap; the
happy path is byte-identical (4 fixtures + v2-convention/fortran/payload-tail pins all green).
Along the way one more hazard was found and removed (D12): `better_assert` on the open failure
prints **and aborts** in assert-enabled builds — the pre-fix missing-file path was a `SIGABRT`
in the suite build — so the boundary now uses the hard check only.

## Decision Log

| Decision | Chosen | Rejected | Reason | Contract Ref |
|---|---|---|---|---|
| D1 | Validation inline in the `load_npy(char const*)` body (single hunk) | Private helper `npy_header_parse` | "diff confined to the function body"; one consumer (YAGNI); in-repo siblings keep validation inline | Deliveries 1 |
| D2 | Keep the library's v2 convention (4B LE length @8, prefix 12) | Real npy v2 spec (8B length, prefix 16); reject v2 | Contract failure mode pins both 10/12 offsets; real-spec adoption = unsanctioned wire change; real-spec v2 files now cleanly rejected via D6 | `failure_modes_to_watch` |
| D3 | Digit-bounded shape parser (define-away) | Keep `stoul` inside try/catch (mask) | Tier-1 beats tier-3; `stoul` accepts `-` (empirically terminate) and is a second throw source | P3; in_scope "stoul wrapped (catch)" intent |
| D4 | Overflow-checked multiply for `row*col` and `payload` | Trust `size_t` width | Third hazard (empirically confirmed pre-fix); mandated non-wrapping style | P3 |
| D5 | One `try/catch(…) → false` around the validated region | Per-site `catch (std::exception)` | With D3/D4 only allocation throws remain, bounded by file size; `noexcept` honest for all inputs; mask at the shell, not the Calculation | in_scope "body stays throw-free" |
| D6 | Header must start with `{` (dict literal) | Reject version 2; byte-swap support | Closes the real-spec-v2 4-byte-shift silent misload without a wire change | P3; row 18 |
| D7 | descr parsed positionally + exact match against canonical descriptor | Substring search for expected dtype | Substring would accept an attacker-planted token outside the descr field | P3 "dtype matches target" |
| D8 | Zero-dim shapes rejected (`row, col ≥ 1`) | Accept `resize(0,·)` | Library non-zero-dim policy (S1 watch item); avoids unverified `resize(0,·)` territory | P3 "row/col ≥ 1" |
| D9 | Byte-level copy via `std::uint8_t*` | Keep strict-typed `copy_n<value_type>` | Identical bytes; removes unaligned strict-typed loads; in-repo pattern (`load_binary`) | house pattern |
| D10 | `row_major` detection preserved verbatim (`header.find("T")`) | Rewrite as `'fortran_order': True` search | Happy-path invariance; with D7+D3 the only `'T'` source is the fortran value; pinned by `e03_fortran` | row 18 "valid files load exactly as before" |
| D11 | `resize` strictly after all checks; tests assert state unchanged | — | Contract failure mode "reject before zen.resize" | `failure_modes_to_watch` |
| D12 | Remove `better_assert(ifs, …)` from `load_npy`; hard check only | Keep the assert as debug message | `print_assertion` calls `abort()` in `debug_mode` builds — red run proved the pre-fix suite build SIGABRTs on a missing file; contract's "unopenable path → clean false" has no mode qualifier | invariants; P2 |
| Path | Handoff to `.work/handoff_session_2.md` | Session protocol's `docs/handoff.md` | `docs/handoff.md` is outside `blast_radius.allowed_files`; project contract §1.4 (higher authority) specifies `.work/handoff_session_{n}.md`; contract wins | blast radius |
| Process | Subagent-style steps (independent derivation, sharded review, adversarial verification) run in-session with disciplined context separation | — | No subagent tool available in this environment (S1 recorded the same model-budget constraint); deviations documented where they occur | protocol |

## Next Priority Queue

1. **Human decision gate for S1+S2** (this branch): review `git diff ad6fa79..HEAD` + the
   evidence chain (suite log, probe outputs, pre-fix `prefix_*`, bug-restoration red); merge on
   sign-off (contract exit 5).
2. **S3 — `fliplr`/`flipud` alias swap** (PRD §6 order; seed E05; S1's review independently
   re-confirmed the inversion — risk register S1 watch item).
3. **I/O-boundary hardening pass (new suggested project):** the S2 watch items found
   `load_binary`/`load_txt` siblings with the same hazard class (overflow-unguarded size
   arithmetic, no dtype/type check) and `better_assert`-abort-on-open; S5's `save_png` is the
   nearest in-flight sibling (warned below).

## S6 doc delta (exact wording — S6 is the ReadMe single writer, P4)

Insert after the code block in ReadMe §"load npy" (~line 1063), verbatim:

> `load_npy` returns `false` without modifying the matrix when the file is truncated,
> malformed, or its stored type does not match the matrix: the dtype in the file must match the
> matrix's type exactly (a float32 file, descr `<f4`, loads into `matrix<float>`; a float64
> file, descr `<f8`, loads into `matrix<double>` — a float32 file is **not** loaded into
> `matrix<double>`), only little-endian dtypes are accepted, and the shape must be a two
> positive-integer pair. Files using the real NPY v2 layout (8-byte header-length field) are
> rejected: this library's v2 convention uses a 4-byte header-length field.

Also fold into S6's ReadMe pass: the R-13 doc-drift check (this is the only S1–S5 delta that
touches user-visible I/O semantics).

## Warnings And Gotchas

- **Environment:** g++ (GCC) 16.2.1 20260810; the suite build has **asserts enabled** (no
  `-DNDEBUG` in the Makefile) — this is what made D12 visible and why the missing-file case is
  pinned in the suite, not only in the `-DNDEBUG` probe. The probe binaries are built ad hoc
  (no Makefile target): `g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1
  .work/probes/E03_E04.cc -o .work/probe_s2`. The untracked `test_test` binary at the repo root
  is a pre-existing S1 build artifact — leave it; `.gitignore` covers `tmp/*` and `.work/`
  (evidence files were force-added to the repo per S1 convention).
- **Known failing tests:** none — full suite green (64/64). The pre-fix red outputs are
  evidence only (`.work/evidence/tdd_red_*.log`).
- **Deferred risks:** S2 watch items in `docs/risk_register.md` (third-hazard class kept in
  rotation; adjacent `load_binary`/`load_txt` hazards — out of scope, do not fix here; v2 wire
  convention disclosure for the ReadMe; `better_assert`-abort pattern still present in the
  other I/O boundaries).
- **Files future sessions must not casually edit:** `ReadMe.md` (S6 single writer, P4);
  `docs/prd.md` + `docs/project_contract.md` (frozen); `tests/test.cc` (case registration —
  not in any fix session's allowed set; the load_npy registration already exists at line 35);
  `docs/risk_register.md` watch items marked "do not fix early"; the pre-fix evidence logs
  (`.work/evidence/prefix_*`) are historical state — do not re-run over them (they document
  the pre-fix tree).
- **S5 warning (per plan):** `save_png` is the only other I/O boundary in S5's blast radius —
  apply the same validate-then-act pattern there (buffer writes: bound-check before every
  `put`/`write`; no `better_assert`-abort on user-reachable failure; no overflow-unguarded
  size arithmetic). The S2 watch-item about `better_assert`-abort-on-open applies.
- **R-02 anchor drift (process):** the S1 review's `load_bmp` anchor (~6760) had drifted to
  6643; all S2 edits were keyed by function name with lines as hints. Contracts/reviews should
  keep citing anchors by name.

## Eval Seeds

- **Promoted (this session):** E03, E04 (`docs/eval_seed_cases.md` — status `seeded` →
  `promoted`; probe `.work/probes/E03_E04.cc`; permanent home `tests/cases/load_npy.hpp`
  negative cases).
- **Missed check:** none outstanding — the two found during sharded review (bad-magic ≥12B,
  zero-dim `(0,2)`) were pinned in the suite before closeout (F1/F2).
- **New regression test candidates (not added — outside S2's seed scope; for the next
  eval-harvest pass):** wrap-product shape `(2^40, 2^24)` (A7); 1 MB junk header (A3);
  directory-as-file-name (A4); v2 12-byte exact-boundary file (A1).
- **Instruction update candidate:** the session-end protocol names `docs/handoff.md` while the
  project contract specifies `.work/handoff_session_{n}.md` — reconcile so future sessions
  don't have to adjudicate (S2 followed the contract; recorded here so the discrepancy is
  explicit, not silent).
