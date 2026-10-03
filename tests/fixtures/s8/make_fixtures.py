#!/usr/bin/env python3
"""S8 (PR-11, PR-14): writes the committed numpy/scipy oracle fixtures for tests/cases/s8_fixtures.hpp.

Run from anywhere: `python3 tests/fixtures/s8/make_fixtures.py` (numpy 2.5.3, scipy 1.18.1). Every input comes
from a fixed seed, so a second run reproduces every .npy file and manifest.txt byte for byte with the same python,
numpy and scipy versions, which the manifest header records.

manifest.txt: comment lines start with '#'; every other line is `<case> <op> <tol> <files...>`, files relative to
this directory. Case names are `<op>_<type>_<shape...>`; the element type is the field after the op and names the
input's dtype: `f4` (<f4), `f8` (<f8), `i4` (<i4), `c8` (<c8) or `c16` (<c16). The tolerance is an absolute bound
on max |got - ref| over the elements (complex modulus for complex elements; 0 means exact).
Ops and their files (all C-order, little-endian):
  conv_full   A B C   C = scipy.signal.convolve2d(A, B, "full")
  conv_same   A B C   C = scipy.signal.convolve2d(A, B, "same")
  conv_valid  A B C   C = scipy.signal.convolve2d(A, B, "valid")
  fft2        A C     C = numpy.fft.fft2(A)     (S8-R2)
  ifft2       A C     C = numpy.fft.ifft2(A)    (S8-R2)
  fftshift    A C     C = numpy.fft.fftshift(A) (S8-R3)
  ifftshift   A C     C = numpy.fft.ifftshift(A) (S8-R3)
For conv f8 the tolerance is 4*eps*sum|A|*sum|B| (S8-R1); i4 is exact. fft2 and ifft2 store C in numpy's result
dtype, which is D-031's: f4 and c8 give c8, f8, i4 and c16 give c16. Their tolerance is 16*L*eps*sum|A| for fft2 and
that divided by N for ifft2 (S8-R2), eps of the result's real type, N = R*C, L = ceil(log2(2N)). The shifts keep
A's dtype and are exact.

Extending (S8-T7): write a generator like conv_cases() that calls add(case, op, tol, [(role, array), ...]) and
call it from main(); the op name keys the dispatch in tests/cases/s8_fixtures.hpp.
"""
import os
import sys

import numpy as np
import scipy
import scipy.signal

HERE = os.path.dirname(os.path.abspath(__file__))
EPS = np.finfo(np.float64).eps

DTYPES = {"f4": "<f4", "f8": "<f8", "i4": "<i4", "c8": "<c8", "c16": "<c16"}


def rng_for(seed):
    return np.random.default_rng(seed)


def real(seed, m, n):
    return rng_for(seed).uniform(-1.0, 1.0, size=(m, n))


def ints(seed, m, n):
    return rng_for(seed).integers(-9, 10, size=(m, n)).astype(np.int32)


def tol_of(x):
    """two significant digits, rounded up, so the manifest text is short and stable; 0 stays 0."""
    return 0.0 if x == 0 else float("%.1e" % (x * 1.05))


# ---- writing ----
LINES = []


def save(case, role, arr, kind):
    arr = np.ascontiguousarray(np.asarray(arr).astype(DTYPES[kind]))
    name = "%s_%s.npy" % (case, role)
    np.save(os.path.join(HERE, name), arr, allow_pickle=False)
    return name


def add(case, op, tol, kind, arrays, inputs=()):
    """one manifest line; `inputs` are already saved file names listed before the files of `arrays`."""
    files = list(inputs) + [save(case, role, arr, kind) for role, arr in arrays]
    LINES.append("%s %s %r %s" % (case, op, tol_of(tol), " ".join(files)))


def shape(a):
    return "%dx%d" % a.shape


# ---- conv (S8-R1) ----
def conv(a, b, kind, tag=""):
    """every mode scipy defines for this pair: valid only when one operand contains the other."""
    modes = ["full", "same"]
    ra, ca = a.shape
    rb, cb = b.shape
    if (ra >= rb and ca >= cb) or (rb >= ra and cb >= ca):
        modes.append("valid")
    tol = 0.0 if kind == "i4" else 4 * EPS * float(np.abs(a).sum()) * float(np.abs(b).sum())
    pair = "%s_%s_%s%s" % (kind, shape(a), shape(b), tag)
    inputs = [save("conv_" + pair, "A", a, kind), save("conv_" + pair, "B", b, kind)]  # shared by the modes
    for mode in modes:
        c = scipy.signal.convolve2d(a, b, mode)
        case = "conv_%s_%s" % (mode, pair)
        add(case, "conv_" + mode, tol, kind, [("C", c)], inputs)


def conv_cases():
    a = real(101, 7, 9)                      # asymmetric A
    conv(a, real(102, 1, 1), "f8")           # 1x1
    conv(a, real(103, 3, 5), "f8")           # odd, asymmetric
    conv(a, real(104, 2, 4), "f8")           # even
    conv(a, real(105, 4, 3), "f8")           # even rows, odd columns
    conv(a, real(106, 3, 12), "f8")          # oversized in columns only: no valid
    conv(a, real(107, 10, 11), "f8")         # oversized in both: valid swaps
    conv(real(108, 1, 6), real(109, 1, 3), "f8")
    conv(real(110, 5, 1), real(111, 2, 1), "f8")
    ai = ints(121, 6, 8)
    conv(ai, ints(122, 1, 1), "i4")
    conv(ai, ints(123, 3, 5), "i4")
    conv(ai, ints(124, 2, 4), "i4")
    conv(ai, ints(125, 9, 10), "i4")


# ---- fft2, ifft2 (S8-R2) and the shifts (S8-R3) ----
FFT_SIZES = [(1, 1), (1, 2), (1, 7), (1, 16), (3, 5), (4, 6), (7, 11), (13, 1), (31, 17), (8, 8), (16, 32), (64, 64)]
SHIFT_SIZES = [(1, 1), (1, 4), (3, 5), (4, 6), (5, 5), (6, 1)]
RESULT_KIND = {"f4": "c8", "c8": "c8", "f8": "c16", "i4": "c16", "c16": "c16"}  # D-031, numpy's own dtypes


def sample(kind, seed, m, n):
    if kind == "i4":
        return ints(seed, m, n)
    if kind in ("f4", "f8"):
        return real(seed, m, n).astype(DTYPES[kind])
    return (real(seed, m, n) + 1j * real(seed + 500, m, n)).astype(DTYPES[kind])


def big_l(n):
    """L = ceil(log2(2N))"""
    l, p = 0, 1
    while p < 2 * n:
        p, l = p * 2, l + 1
    return l


def fourier(kind, m, n, seed):
    a = sample(kind, seed, m, n)
    out = RESULT_KIND[kind]
    eps = float(np.finfo(np.float32 if out == "c8" else np.float64).eps)
    tol = 16 * big_l(m * n) * eps * float(np.abs(a.astype(np.complex128)).sum())
    tag = "%s_%dx%d" % (kind, m, n)
    inputs = [save("fourier_" + tag, "A", a, kind)]  # shared by fft2 and ifft2
    f, i = np.fft.fft2(a), np.fft.ifft2(a)
    assert f.dtype == np.dtype(DTYPES[out]) and i.dtype == np.dtype(DTYPES[out]), (kind, f.dtype, i.dtype)
    add("fft2_" + tag, "fft2", tol, out, [("C", f)], inputs)
    add("ifft2_" + tag, "ifft2", tol / (m * n), out, [("C", i)], inputs)


def fourier_cases():
    for k, (m, n) in enumerate(FFT_SIZES):
        fourier("f8", m, n, 201 + k)
    for k, (m, n) in enumerate([(1, 1), (1, 7), (3, 5), (4, 6), (7, 11), (8, 8), (31, 17)]):
        fourier("c16", m, n, 231 + k)
    for k, (m, n) in enumerate([(1, 2), (1, 16), (3, 5), (7, 11), (13, 1), (16, 32)]):
        fourier("f4", m, n, 251 + k)
    for k, (m, n) in enumerate([(3, 5), (8, 8)]):
        fourier("c8", m, n, 271 + k)
    for k, (m, n) in enumerate([(1, 7), (4, 6), (7, 11), (8, 8)]):
        fourier("i4", m, n, 281 + k)


def shift_cases():
    for j, kind in enumerate(["f8", "i4", "c16"]):
        for k, (m, n) in enumerate(SHIFT_SIZES):
            a = sample(kind, 301 + 10 * j + k, m, n)
            tag = "%s_%dx%d" % (kind, m, n)
            inputs = [save("shift_" + tag, "A", a, kind)]
            add("fftshift_" + tag, "fftshift", 0.0, kind, [("C", np.fft.fftshift(a))], inputs)
            add("ifftshift_" + tag, "ifftshift", 0.0, kind, [("C", np.fft.ifftshift(a))], inputs)


def main():
    for f in os.listdir(HERE):
        if f.endswith(".npy"):
            os.remove(os.path.join(HERE, f))

    conv_cases()
    fourier_cases()
    shift_cases()

    header = [
        "# S8 oracle fixtures, written by tests/fixtures/s8/make_fixtures.py; do not edit by hand.",
        "# python %s, numpy %s, scipy %s" % (sys.version.split()[0], np.__version__, scipy.__version__),
        "# <case> <op> <tol> <files...>",
    ]
    with open(os.path.join(HERE, "manifest.txt"), "w", newline="\n") as f:
        f.write("\n".join(header + LINES) + "\n")


if __name__ == "__main__":
    main()
