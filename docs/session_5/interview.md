# Session 5 — Interview (self stress-test)

Per the S4 precedent: the "interview" is a self stress-test of the session plan's
ambiguities before brainstorming. Confidence must reach ≥95% on every load-bearing
decision before the execution contract is written. All claims below are re-verified
against the current source (7877-line `matrix.hpp`, HEAD `c40b04b`) during pre-flight.

## Framing

Session 5 = robustness: four findings (C11, C12, S2-finding/R3-slice, C7). Machine
authority: `docs/session_5_contract.yaml` (risk medium, blast radius = `matrix.hpp`,
`tests/test.cc`, `tests/cases/rand.hpp`, `.work/`, `docs/eval_seed_cases.md`,
`docs/risk_register.md`, `docs/session_5/**`). No backward compatibility.

## Question log (stress-test of residual points)

### Q1 — C11: what exactly does "not global state" require, and what happens to seed 0?

**Ambiguity.** The plan says "per-call engine" but not which engine, and says seed 0
"keeps the documented behavior (time-based, R-05 note)" while the review claims
"two seed-0 `rand` calls in the same second → identical matrices".

**Resolution.** (a) Engine: `std::mt19937` seeded with the same `unsigned int` the
pre-fix code computed, wrapped in `std::uniform_real_distribution<T>` exactly as the
contract prescribes (`matrix.hpp` already includes `<random>` at line 29 — no new
include). (b) Seed-0 behavior: keep the *exact* pre-fix expression
`static_cast<unsigned int>(time + reinterpret_cast<uint64_t>(&ans))`. The
"same-second identical" claim is **refined by ltrace evidence** (see Q7): the seed is
stable *per call site* (two adjacent locals differ by the 32-byte stack distance), so
repeated seed-0 calls at one call site within one second return the same matrix;
across call sites the address salt usually differs. The residual per-call-site
correlation is inherent to a time-based seed and is **documented, not violated** — the
contract explicitly permits keeping time-based seeding. What C11 removes is the *global
shared generator* (data race + cross-thread coupling), not the low-entropy seed.
**Confidence: 97%.**

### Q2 — C11: does the suite get a red?

**Ambiguity.** TDD expects a failing test first. But explicit-seed determinism
(rand(…,7) twice equal), seed inequality, and the [0,1) range all **hold pre-fix**
(verified: `seed 7 == 7: 1`, `seed 7 != 8: 1`, P0 log). There is no value-level red.

**Resolution.** The executable red for C11 is *structural*, and the contract itself
frames it that way: `grep 'srand\|std::rand' == 0` + the E14 invariant pin + the
thread-safety check. Concretely:
- **red (pre-fix):** `grep -cE 'srand\(|std::rand\(' matrix.hpp` = **3** (lines
  5327/5329/5333) — global mutable generator state;
- **green (post-fix):** same grep = **0**; TSan post-fix run clean (no user-code
  races); E14 suite case green pre-*and* post-fix (invariant pin);
- the pre-fix P0 correlation demo (100 same-site seed-0 calls → 1 distinct value)
  remains *by design* post-fix (same seed → same `mt19937` stream) — the fix is
  removal of global state, not removal of the seed-0 correlation. Documented.
**Confidence: 98%.**

### Q3 — C11: `noexcept` fallout beyond `rand`?

**Ambiguity.** The contract's change list says "drop `noexcept` from `rand` and
`rand_like`". But the *current* source has `noexcept` on **six** declarations in the
rand chain: `rand` (5322), `rand_like` (5353), `random_like` (5360), `randn_like`
(5365) — plus `random(r,c)`/`random(n)` (5344/5349) are *not* noexcept and call
`rand`. Once `rand` can throw (allocation inside `std::generate`/matrix ctor), any
`noexcept` wrapper that transitively calls it becomes a `std::terminate` trap.

**Resolution.** Drop `noexcept` from the **entire chain**: `rand` (5323),
`rand_like` (5355), `random_like` (5361), `randn_like` (5366). (`random(r,c)`/`random(n)`
at 5345/5350 are already non-noexcept and need no change.) This is a direct consequence of the sanctioned engine
change (same defect the contract names for `rand_like`), not alias rationalization —
S6 still owns the *renaming* of `random`/`random_like`/`randn_like`; the noexcept
state they inherit is correct after S5. Boundary recorded in handoff + risk register.
**Confidence: 96%.**

### Q4 — C11: `T = int` and `T = complex` instantiations?

**Ambiguity.** The contract pins "int instantiation: sane **or documented**".

**Resolution.** With `uniform_real_distribution<T>` + `static_cast<T>(…)`:
- `int`: `(rand()+1)/(RAND_MAX+2)` was *already* always 0 pre-fix for int (integer
  division: `rand()+1 ≤ RAND_MAX+1 < RAND_MAX+2`), and `static_cast<int>([0,1))` is
  always 0 post-fix. Behavior **unchanged**, documented as such (suite pins it).
- `complex`: `uniform_real_distribution<std::complex<…>>` is not a valid
  distribution → complex-T `rand` **no longer compiles**. In-repo consumers: **none**
  (all call sites use `double`/`float`). The contract prescribes the distribution
  type, so this is sanctioned by construction; documented in handoff + risk register.
**Confidence: 97%.**

### Q5 — C12: the plan says "unguarded `total_cores` at 1152" — is the second site really unguarded?

**Ambiguity.** The plan lists two sites (1152 and 4121) as unguarded.

**Resolution (discrepancy found, logged).** Re-read of source:
- **1152** (`reduce`): `unsigned int const total_cores = std::thread::hardware_concurrency();`
  → divides at 1163. **Unguarded** — SIGFPE if 0. Confirmed.
- **4121** (`reduce_impl_private`): `auto parallel_size = std::thread::hardware_concurrency();`
  → immediately short-circuits: `if ( parallel_size <= 1 || mat.size() < 32 )` takes
  the sequential path. **Already guarded against 0** (0 ≤ 1). The plan's "unguarded"
  claim for 4121 is **not supported by the source**. The contract still *mandates*
  the guard at 4121 (`parallel_size < 1` + grep acceptance) → add the explicit clamp
  anyway (belt-and-suspenders; zero behavior change; satisfies the acceptance grep).
  Logged as a plan/contract discrepancy in the risk register (S5 watch item).
**Confidence: 98%.**

### Q6 — Acceptance greps as written are unsatisfiable or false-positive. Refinements?

**Ambiguity.** Two of the four acceptance greps cannot be met literally.

**Resolution (P9-style clarifications, logged — contract intent preserved):**
1. `grep 'srand\|std::rand' matrix.hpp == 0` **false-positives** on
   `std::random_access_iterator_tag` (line 151). Intent = "no C generator calls".
   Refined: `grep -cE 'srand\(|std::rand\(' matrix.hpp == 0` (pre-fix = 3, post-fix = 0).
2. `grep 'total_cores < 1' >= 3` is **unachievable**: the file has exactly two
   `total_cores`-style sites; line 279 already uses `total_cores <= 1` (the contract
   itself says leave 279 unchanged); 1152 gets `total_cores < 1`; the 4121 variable is
   named `parallel_size`. Refined: `grep -c 'total_cores < 1' == 1` **and**
   `grep -c 'parallel_size < 1' == 1` (both new clamps present) **and** the 279
   `total_cores <= 1` line unchanged.
3. `grep 'if (!fp) return' >= 1` — satisfiable as written (guard at ~3184).
4. `grep 'parallel_size < 1' == 1` — satisfiable as written (new guard at 4121).
**Confidence: 98%.**

### Q7 — Pre-flight findings (evidence, not narrative)

1. **Baseline green:** `make test` → 73 cases / 49,217,182 assertions, all pass
   (`s5_baseline_test.log`); live seeds E01–E13 all PASS.
2. **srand sites:** exactly 3 (5327/5329/5333), all inside `rand`. No other
   `srand`/`std::rand` consumers in the file.
3. **ltrace on the pre-fix seed:** two stable seed values per probe run
   (e.g. `3067854750` / `3067854782`), exactly 32 bytes apart (the `&ans` stack
   distance between the two call sites), constant across iterations within the same
   second. Same-site 100 seed-0 calls → **1 distinct first-element value** (perfect
   per-call-site correlation). Cross-site: different. → Q1 refinement confirmed.
4. **Pre-fix E15 is an executable red:** `save_as_png` to an unwritable path →
   **SIGSEGV (exit 139)** (null `FILE*` UB, line 3182 unchecked `fopen`).
5. **TSan pre-fix = clean — documented limitation.** The race lives in libc's
   internal `rand()` state (uninstrumented); TSan only sees accesses from
   instrumented code, so it reports nothing pre-fix. This is the contract's own
   fallback clause ("otherwise reasoning + grep for absence of global state"):
   C11's thread-safety proof = global state removed (grep 0) + local engine by
   construction + TSan post-fix (no *new* user-code races).
6. **`<random>` already included** (line 29) — no header change needed.
7. **Catch v2.0.1 quirk** (S4-learned): `REQUIRE(a && b)` with temporaries can fail
   to compile — suite cases use local bools + separate `REQUIRE`s.
**Confidence: 99%.**

### Q8 — C12 and save_png: why no suite cases?

**Resolution.** Contract: "no new test cases for C12 or save_png — E15 is a
probe-only case". C12 cannot be forced to `hardware_concurrency() == 0` in this
environment (it reads the affinity-adjusted online-CPU count; `taskset` yields 1,
not 0) → evidence = grep + code review. save_png's happy path is pinned by the E15
probe's positive control (writable path → PNG exists); a full PNG-content suite case
is not required by the contract. The stray `;;` (line 3190) is a harmless double
semicolon — removal is the sanctioned R3-slice, verified by diff + rebuild.
**Confidence: 99%.**

### Q9 — C7: what is the deliverable?

**Resolution.** No code. The `NDEBUG` policy delta is **authored text** for S6's
ReadMe change (ReadMe.md is out of scope here). Mechanism pinned from source:
`debug_mode` (lines 57–61) is `constexpr 0` when `NDEBUG` is defined;
`print_assertion` prints to `std::cerr` and `abort()`s **only** in debug mode; under
`NDEBUG`, `better_assert` is a silent no-op. The delta text also records the
in-repo I/O-boundary precedent set by S2 (`load_npy`) and S5 (`save_png`): hard,
NDEBUG-independent, silent-failure checks at I/O boundaries vs debug-only
`better_assert` preconditions. Full draft in `specs/ndebug_policy.md` + `design.md §4`.
**Confidence: 99%.**

### Q10 — R-07 (sanctioned random stream change): how is it evidenced?

**Resolution.** Examples 0012/0019/0020/0021 call `rand`/`random` with explicit
seeds (1/2) or seedless; 0011/0008/0006/0009/0013 are seedless (values change every
run even pre-fix). Post-fix: pre-fix value streams recorded (P0 log: `rand(1,4,1)` =
`0.84018771683788496 …`); `make example` run at closeout, stdout delta recorded in
`.work/evidence/s5_example_delta.txt`, `images/` checked out (`git checkout --
images/` policy, same as S4).
**Confidence: 99%.**

## Residual risks (accepted, logged)

| # | Risk | Disposition |
|---|------|-------------|
| 1 | Seed-0 per-call-site correlation persists by design (time-based) | Documented (contract permits time-based); risk register |
| 2 | Explicit-seed streams change (R-07) | Sanctioned; `make example` delta recorded |
| 3 | Complex-T `rand` no longer compiles | No in-repo consumers; documented (contract-prescribed distribution) |
| 4 | `rand_like`/`random*` noexcept dropped beyond the contract's two named | Same-defect extension, boundary documented; S6 inherits correct state |
| 5 | Plan's 4121 "unguarded" claim unsupported | Already guarded by short-circuit; explicit clamp added per contract; discrepancy logged |
| 6 | `load_binary` adjacent `fopen` warning (S2 handoff) | Out of scope (not in contract blast radius); carried to risk register only |

## Stop conditions (project contract §1.3)

Stop and surface: any required behavior change beyond the blast radius; a contract
line unsatisfiable as written (the two grep refinements in Q6 are *refinements of
intent-preserving wording*, not behavior changes — logged, not stoppers); an
environment failure blocking a required check (record ENVIRONMENT evidence first).

**Final confidence: ≥97% on all load-bearing decisions → proceed to brainstorming.**
