# S10 benchmark results

The measured optimizations of stage S10 (S10-R3, D-008). Each kept optimization is listed in `bench/kept.txt`
under the same id and is judged by the compare lane on every full run.

## How to run

- `tools/check.sh bench compare`: full run, every workload of `bench --list`; judges kept optimizations and
  regressions; ends with `LANE bench PASS: …` or `LANE bench FAIL: …`. About 145 s on the host below with the
  parallel variant (95 s serial only).
- `tools/check.sh bench compare --smoke`: one small size per family plus the kept workloads; kept optimizations
  are judged, regressions are printed but not judged.
- `BENCH_CPU=<n>` sets the CPU the runs are pinned to (default 2); `BENCH_ONLY=<regex>` restricts the workloads,
  for experiments only (a run that leaves out a kept workload cannot pass).
- Logs go to `build/logs/bench/`; binaries to `build/bench/compare/{base,head,base-par,head-par}/bench`.

## Method

- Baseline: `matrix.hpp` at the parent of the oldest commit whose subject is exactly `S10: spec and tasks`,
  extracted with `git show`; head: the working tree. Both builds compile the same `bench/bench.cc` with the
  first GCC and exactly `-std=c++20 -O2 -pthread` (no -O3, -march=native or fast-math, D-006).
- Inputs come from `std::mt19937_64` seeded with a constant plus a hash of the workload name.
- Each process warms up first: doubling a batch of operations until one batch lasts at least 5 ms (1 ms smoke).
  Each sample is then one batch, in seconds per operation.
- Interleaving: per workload the lane runs 8 processes per build in the order ABBA BAAB ABBA BAAB (A base,
  B head), 5 samples each, pinned with `taskset -c $BENCH_CPU`; the 40 samples of each build are pooled
  (smoke: ABBA, 3 samples each, 6 pooled).
- Per build: median and rel. dispersion = median absolute deviation / median. Change = head median / base
  median − 1. A workload is stable when its rel. dispersion is at most 5% in both builds (D-035).
- Judgement (D-008): every workload of a kept optimization has change ≤ −10%; no stable workload has change
  > +5%; unstable workloads are printed with their numbers and not judged.

## Host

From the INFO lines of the full run below (head at commit c8b3206, baseline 78dc886, the parent of 3cbad44):

- CPU: AMD Ryzen 9 7900X3D 12-Core Processor, 24 cores; pinned to cpu 2 (`taskset -c 2`); par/ workloads
  unpinned
- Compiler: g++ 16.2.1 20260810 (all four builds); flags `-std=c++20 -O2 -pthread` (base, head) and
  `-std=c++20 -O2 -pthread -DFENG_MATRIX_PARALLEL` (base-par, head-par)
- Mode: full, order ABBA BAAB ABBA BAAB, 8 runs x 5 samples = 40 pooled per build
- Result: `LANE bench PASS: 3 kept, 102 workloads, 6 unstable` (the six: conv/64k5, par/gemm/256,
  par/gemm/1024, par/gemm/wide and the kept par/gemm/64 and par/gemv/1024); largest stable change outside the
  kept workloads: par/map/4x4 +3.42%

The tables below come from this run. Medians in seconds per operation; rel. dispersion base / head in %; label
`stable` when both are at most 5% (D-035).

## Kept optimizations

`gemm-blocked` (D-036): `operator*=`, `operator*` and `direct_multiply` use one cache-blocked row-major i-k-j
kernel (`matrix_details::gemm_blocked`) in serial and parallel builds, in place of the strided
`std::inner_product` per entry and, in serial builds, Strassen recursion for max dimension ≥ 17. Each entry is
the k-ascending sum from zero, bit-identical to the old kernel kept as `matrix_details::gemm_reference`
(`[S10][S10-R3]` tests).

| id | workload | baseline median | head median | change % | dispersion % | label |
|---|---|---|---|---|---|---|
| gemm-blocked | gemm/8 | 2.5209e-07 | 1.5264e-07 | -39.45 | 1.34 / 2.07 | stable |
| gemm-blocked | gemm/16 | 1.5914e-06 | 9.7036e-07 | -39.02 | 0.38 / 0.40 | stable |
| gemm-blocked | gemm/17 | 3.4108e-06 | 1.1400e-06 | -66.58 | 0.81 / 0.39 | stable |
| gemm-blocked | gemm/18 | 6.5067e-06 | 2.2080e-06 | -66.07 | 0.67 / 0.55 | stable |
| gemm-blocked | gemm/32 | 1.4983e-05 | 7.2298e-06 | -51.75 | 0.24 / 0.42 | stable |
| gemm-blocked | gemm/64 | 1.7852e-04 | 5.7231e-05 | -67.94 | 0.92 / 0.19 | stable |
| gemm-blocked | gemm/128 | 1.2685e-03 | 4.5228e-04 | -64.34 | 0.57 / 0.18 | stable |
| gemm-blocked | gemm/256 | 8.4660e-03 | 3.9624e-03 | -53.20 | 0.26 / 0.48 | stable |
| gemm-blocked | gemm/512 | 5.7637e-02 | 3.0558e-02 | -46.98 | 0.51 / 0.36 | stable |
| gemm-blocked | gemm/1024 | 3.9512e-01 | 2.3764e-01 | -39.86 | 0.61 / 0.75 | stable |
| gemm-blocked | gemm/tall | 7.5342e-02 | 4.9836e-03 | -93.39 | 1.64 / 1.75 | stable |
| gemm-blocked | gemm/wide | 6.2296e-02 | 4.0113e-03 | -93.56 | 1.10 / 2.94 | stable |
| gemm-blocked | gemv/8 | 5.5061e-08 | 3.8638e-08 | -29.83 | 1.79 / 0.96 | stable |
| gemm-blocked | gemv/16 | 1.4597e-07 | 8.6724e-08 | -40.59 | 1.85 / 1.93 | stable |
| gemm-blocked | gemv/32 | 4.7085e-07 | 2.6251e-07 | -44.25 | 0.94 / 1.56 | stable |
| gemm-blocked | gemv/64 | 2.1021e-06 | 1.1171e-06 | -46.86 | 0.20 / 0.10 | stable |
| gemm-blocked | gemv/128 | 9.3135e-06 | 4.6953e-06 | -49.59 | 0.21 / 0.22 | stable |
| gemm-blocked | gemv/256 | 4.3272e-05 | 1.9614e-05 | -54.67 | 0.29 / 0.37 | stable |
| gemm-blocked | gemv/512 | 1.8907e-04 | 8.6784e-05 | -54.10 | 0.41 / 1.09 | stable |
| gemm-blocked | gemv/1024 | 7.8252e-04 | 3.4892e-04 | -55.41 | 0.47 / 1.40 | stable |

### par-thresholds

`par-thresholds` (S10-T7): in `FENG_MATRIX_PARALLEL` builds the default worker count is work-based,
`matrix_details::work_workers( work, grain )` = min( hardware_concurrency, work / grain ), at least 1, in place of
`hardware_concurrency` workers for every elementwise call (threshold 0), for `for_each` above 1024 elements and for
reductions above 32 elements. Grains: 2^16 elements for elementwise calls (`+=`, `-=`, `+`, `-`, unary minus,
`copy`, `clone`, `meshgrid`, `save_as_bmp` rows), callbacks (`for_each`, the maps, `apply`, colormaps, `pooling`)
and reductions (`sum`, `min`, `max`, `reduce`); 2^18 multiply-adds (M·K·N) for products, whose rows are split over
at most M workers. Explicit worker counts and serial builds are unchanged; the 1-worker run is the fallback,
and `[S10][S10-R3]` "thresholded parallel helpers equal the serial path" shows elementwise results bit-identical
to it, reductions equal to `reduce_range` with the same worker count and products bit-identical to
`gemm_reference`.

How the grains were chosen: a scratch program timed `parallel_workers`, `reduce_range` and `gemm_blocked` at
1, 2, 3, 4, 6, 8, 12, 16 and 24 workers (median of 15 batches each, warm inputs, -O2). Starting and joining one
`std::jthread` costs about 19 µs, so at 24 workers any call costs about 0.45 ms. One worker is fastest up to
about 2^15 elements for sqrt via `for_each` (54 µs vs 58 µs on 2), 2^16 for sum (38 µs vs 49 µs) and 2^18
multiply-adds for GEMM (64^3: 51 µs vs 58 µs on 3). At 2^17 elements sqrt is 211 µs on 1 worker and 111 µs on 4;
96^3 GEMM is 171 µs on 1 and 105 µs on 4. The elementwise grain comes from the workload itself: par/add/1000x1000
(a + b into a fresh result), three interleaved runs each, gave 0.75–0.89 ms at grain 2^16 (15 workers),
0.84–0.90 ms at 2^17 (7), 0.98–1.06 ms at 2^18 (3) and 0.98–1.06 ms at the baseline's 24. For par/map/1000x1000 a
callback grain of 2^16 (15 workers, 0.92–0.95 ms) beat 2^13–2^15 (24 workers, 1.00–1.09 ms).

par/gemm/256, par/gemm/1024, tall and wide are not listed: their worker count is 24 in both builds, so their gains
come from `gemm-blocked`. par/map/1000x1000 is closest to the limit (−14.00 %). par/gemm/64 and par/gemv/1024 are
unstable in this run (baseline dispersion 5.25 %); both are far beyond the 10 % bar.

| id | workload | baseline median | head median | change % | dispersion % | label |
|---|---|---|---|---|---|---|
| par-thresholds | par/add/4x4 | 3.0267e-04 | 1.6460e-08 | -99.99 | 4.08 / 1.24 | stable |
| par-thresholds | par/add/32x32 | 4.7099e-04 | 2.7403e-07 | -99.94 | 2.50 / 1.25 | stable |
| par-thresholds | par/add/100x100 | 4.7990e-04 | 3.2316e-06 | -99.33 | 2.96 / 1.79 | stable |
| par-thresholds | par/add/1000x1000 | 8.4921e-04 | 6.7131e-04 | -20.95 | 2.16 / 2.08 | stable |
| par-thresholds | par/map/100x100 | 4.7906e-04 | 1.6933e-05 | -96.47 | 2.07 / 0.71 | stable |
| par-thresholds | par/map/1000x1000 | 9.6448e-04 | 8.2944e-04 | -14.00 | 1.82 / 1.20 | stable |
| par-thresholds | par/sum/32x32 | 4.6266e-04 | 5.5930e-07 | -99.88 | 3.38 / 0.70 | stable |
| par-thresholds | par/sum/100x100 | 4.5574e-04 | 5.8643e-06 | -98.71 | 2.82 / 1.35 | stable |
| par-thresholds | par/sum/1000x1000 | 4.7748e-04 | 3.0659e-04 | -35.79 | 1.81 / 1.94 | stable |
| par-thresholds | par/gemm/8 | 1.3027e-04 | 1.3194e-07 | -99.90 | 1.76 / 1.02 | stable |
| par-thresholds | par/gemm/16 | 2.8894e-04 | 8.9055e-07 | -99.69 | 0.98 / 0.81 | stable |
| par-thresholds | par/gemm/17 | 3.0637e-04 | 1.0283e-06 | -99.66 | 1.93 / 1.13 | stable |
| par-thresholds | par/gemm/18 | 3.3770e-04 | 1.1884e-06 | -99.65 | 2.70 / 1.35 | stable |
| par-thresholds | par/gemm/32 | 4.8804e-04 | 6.6837e-06 | -98.63 | 4.43 / 1.13 | stable |
| par-thresholds | par/gemm/64 | 5.0838e-04 | 5.2863e-05 | -89.60 | 5.25 / 1.06 | unstable |
| par-thresholds | par/gemv/8 | 1.3267e-04 | 3.3619e-08 | -99.97 | 1.72 / 2.87 | stable |
| par-thresholds | par/gemv/32 | 4.8237e-04 | 2.5033e-07 | -99.95 | 4.78 / 1.64 | stable |
| par-thresholds | par/gemv/64 | 4.8741e-04 | 1.0649e-06 | -99.78 | 4.60 / 1.66 | stable |
| par-thresholds | par/gemv/1024 | 9.5979e-04 | 5.7155e-04 | -40.45 | 5.25 / 3.93 | unstable |

Rows of this run outside the kept workloads with change ≥ +2 %, all stable and in code paths the kept changes
leave serial or untouched: par/map/4x4 +3.42 % (16 elements, one worker in both builds), map/1000x1000 +2.31 %,
scale/4x4 +2.08 %. In the run before `par-thresholds` par/map/4x4 was +1.54 %; these rows move by a few percent
between runs (D-035).

### fft-plans

`fft-plans` (S10-T8, B-042): `matrix_details::fft2` builds one `fft_plan` per length per call and reuses it for
every row (length C) and every column (length R; the row plan when R == C). A radix-2 plan holds the twiddle
table for the sign used; a Bluestein plan for length n holds the chirp w (n), the forward radix-2 transform of b
(length M) and the length-M twiddle tables for -1 and +1, plus the length-M scratch. The per-row path that
rebuilt all of these for each row and column (three table builds and the chirp transform per Bluestein call) is
kept as `matrix_details::fft2_reference`; every plan value comes from the same `std::polar` expressions in the
same order, and `[S10][S10-R3]` "fft with reused plans equals the per-row transform" shows fft and ifft
bit-identical to it (int, float, double, complex<double>; 0x0 to 250x3).

| id | workload | baseline median | head median | change % | dispersion % | label |
|---|---|---|---|---|---|---|
| fft-plans | fft/128 | 3.4088e-04 | 2.0700e-04 | -39.28 | 1.67 / 2.14 | stable |
| fft-plans | fft/256 | 1.6610e-03 | 1.1714e-03 | -29.48 | 1.20 / 0.44 | stable |
| fft-plans | fft/125 | 2.4250e-03 | 8.7432e-04 | -63.95 | 0.87 / 3.62 | stable |
| fft-plans | fft/250 | 1.0221e-02 | 3.8516e-03 | -62.32 | 0.80 / 3.08 | stable |

## Parallel variant

The lane also builds both headers with `-std=c++20 -O2 -pthread -DFENG_MATRIX_PARALLEL` (`base-par`, `head-par`).
Their workloads are named `par/<workload>`: elementwise (add, scale, map) and statistics (sum, mean, variance,
max) at all four sizes, gemm 8/16/17/18/32/64/256/1024/tall/wide and gemv 8/32/64/1024. They use the same
inputs, interleave and rules as the serial rows but run unpinned (they need many cores). Starting point for the
threshold work (before `par-thresholds`), from one full run (head 9f75ba6 plus this lane, baseline 78dc886; `LANE bench PASS: 1 kept, 102
workloads, 5 unstable`). Medians in seconds per operation; rel. dispersion base / head in %.

| workload | baseline median | head median | change % | dispersion % |
|---|---|---|---|---|
| par/add/4x4 | 3.1543e-04 | 3.1329e-04 | -0.68 | 5.40 / 6.26 |
| par/scale/4x4 | 1.4137e-08 | 1.3950e-08 | -1.32 | 3.31 / 1.05 |
| par/map/4x4 | 2.7156e-08 | 2.7574e-08 | +1.54 | 2.10 / 1.34 |
| par/sum/4x4 | 4.4818e-09 | 4.2869e-09 | -4.35 | 1.81 / 1.39 |
| par/add/32x32 | 4.6794e-04 | 4.6743e-04 | -0.11 | 3.19 / 3.16 |
| par/sum/32x32 | 4.5935e-04 | 4.5610e-04 | -0.71 | 2.61 / 1.77 |
| par/add/100x100 | 4.7078e-04 | 4.6572e-04 | -1.07 | 1.91 / 2.21 |
| par/map/100x100 | 4.7225e-04 | 4.7453e-04 | +0.48 | 2.55 / 1.56 |
| par/add/1000x1000 | 9.4195e-04 | 9.7350e-04 | +3.35 | 7.26 / 6.49 |
| par/gemm/8 | 1.3303e-04 | 1.3789e-04 | +3.65 | 3.42 / 6.01 |
| par/gemm/64 | 4.9291e-04 | 4.7936e-04 | -2.75 | 3.51 / 2.76 |
| par/gemm/256 | 1.5610e-03 | 7.3545e-04 | -52.89 | 3.05 / 1.09 |
| par/gemm/1024 | 2.1967e-01 | 2.4593e-02 | -88.80 | 4.87 / 7.57 |
| par/gemv/8 | 1.3303e-04 | 1.3271e-04 | -0.24 | 3.09 / 3.54 |

What the numbers show: a call that reaches the parallel loop costs 0.13–0.5 ms however small it is (add at every
size, map from 100x100, sum from 32x32, gemm and gemv at every size up to 64; par/add/4x4 is 3.1e-04 s, serial
add/32x32 3.2e-07 s against par/add/32x32 4.7e-04 s); scale, mean, variance, max and the 4x4 statistics stay at
serial cost. In the parallel build the head GEMM gains 53% at 256 and 89% at 1024. The full table is in the lane
output (`build/logs/bench/compare.log`). These are the numbers before `par-thresholds` (above), which is now
listed in `bench/kept.txt`.

## Rejected candidates

None yet.

## Unstable workloads

From the full run above, not judged (rel. dispersion base / head):

- conv/64k5: baseline 4.2579e-05 s, head 4.0150e-05 s, change −5.70 %, 10.07 % / 5.13 %
- par/gemm/256: baseline 1.9032e-03 s, head 7.7015e-04 s, change −59.53 %, 14.46 % / 3.20 %
- par/gemm/1024: baseline 2.3209e-01 s, head 2.7134e-02 s, change −88.31 %, 7.17 % / 10.15 %
- par/gemm/wide: baseline 9.7876e-03 s, head 8.5125e-04 s, change −91.30 %, 5.52 % / 2.59 %
- par/gemm/64 and par/gemv/1024 (kept, `par-thresholds` table above)
