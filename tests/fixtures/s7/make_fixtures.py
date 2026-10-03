#!/usr/bin/env python3
"""S7-R6 (PR-10, PR-14): writes the committed numpy/scipy oracle fixtures for tests/cases/s7_r6.hpp.

Run from anywhere: `python3 tests/fixtures/s7/make_fixtures.py`. Every input comes from a fixed seed, so a second
run reproduces every .npy file and manifest.txt byte for byte (with the same python, numpy and scipy versions,
which the manifest header records).

manifest.txt: comment lines start with '#'; every other line is `<case> <op> <tol> <files...>`, files relative
to this directory. Case names are `<op>_<f8|c16>_<shape>`; the element type is the second field of the name.
Ops and their files (all C-order, little-endian `<f8` or `<c16`):
  lu_solve  A B X   X = numpy.linalg.solve(A, B)
  det       A D     D is 1x1: numpy.linalg.det(A), or the exact fractions.Fraction value for a singular A
  inv       A X     X = numpy.linalg.inv(A)
  svdvals   A S     S is 1-D, numpy.linalg.svd(A, compute_uv=False), descending
  pinv      A X     X = numpy.linalg.pinv(A, rtol=max(m, n)*eps)
  cholesky  A L     L = numpy.linalg.cholesky(A), lower
  rref      A R     R = the exact RREF of A by fractions.Fraction elimination (Gaussian rationals for c16)
  expm      A E     E = scipy.linalg.expm(A)
The test compares ||got - ref||_F / ||ref||_F (||got - ref||_F when ref is 0) with tol.
"""
import os
import sys
from fractions import Fraction

import numpy as np
import scipy
import scipy.linalg

HERE = os.path.dirname(os.path.abspath(__file__))
EPS = np.finfo(np.float64).eps


def rng_for(seed):
    return np.random.default_rng(seed)


def real(seed, m, n):
    return rng_for(seed).uniform(-1.0, 1.0, size=(m, n))


def cplx(seed, m, n):
    g = rng_for(seed)
    return g.uniform(-1.0, 1.0, size=(m, n)) + 1j * g.uniform(-1.0, 1.0, size=(m, n))


def well(a):
    """a + n*I: a well-conditioned square matrix."""
    return a + a.shape[0] * np.eye(a.shape[0])


def low_rank(seed, m, n, r, complex_=False):
    """an m x n matrix of exact rank r with small-integer entries (products are exact in binary64)."""
    g = rng_for(seed)
    b = g.integers(-3, 4, size=(m, r)).astype(np.float64)
    c = g.integers(-3, 4, size=(r, n)).astype(np.float64)
    if complex_:
        b = b + 1j * g.integers(-3, 4, size=(m, r))
        c = c + 1j * g.integers(-3, 4, size=(r, n))
    return b @ c


def cond(a):
    s = np.linalg.svd(a, compute_uv=False)
    s = s[s > max(a.shape) * EPS * s[0]]
    return float(s[0] / s[-1])


def tol_of(x):
    """two significant digits, rounded up, so the manifest text is short and stable."""
    return float("%.1e" % (x * 1.05))


# ---- exact elimination with Fractions (Gaussian rationals for complex) ----
class Q:
    """a + b i with Fraction parts."""

    __slots__ = ("re", "im")

    def __init__(self, re, im=Fraction(0)):
        self.re, self.im = Fraction(re), Fraction(im)

    def __sub__(self, o):
        return Q(self.re - o.re, self.im - o.im)

    def __mul__(self, o):
        return Q(self.re * o.re - self.im * o.im, self.re * o.im + self.im * o.re)

    def __truediv__(self, o):
        d = o.re * o.re + o.im * o.im
        return Q((self.re * o.re + self.im * o.im) / d, (self.im * o.re - self.re * o.im) / d)

    def iszero(self):
        return self.re == 0 and self.im == 0


def to_q(a):
    return [[Q(Fraction(float(np.real(x))), Fraction(float(np.imag(x)))) for x in row] for row in a]


def exact_rref(a):
    m = to_q(a)
    rows, cols = len(m), len(m[0])
    r = 0
    for c in range(cols):
        p = next((i for i in range(r, rows) if not m[i][c].iszero()), None)
        if p is None:
            continue
        m[r], m[p] = m[p], m[r]
        piv = m[r][c]
        m[r] = [x / piv for x in m[r]]
        for i in range(rows):
            if i != r and not m[i][c].iszero():
                f = m[i][c]
                m[i] = [x - f * y for x, y in zip(m[i], m[r])]
        r += 1
        if r == rows:
            break
    out = np.array([[complex(float(x.re), float(x.im)) for x in row] for row in m])
    return out if np.iscomplexobj(a) else out.real.copy()


def exact_det(a):
    m = to_q(a)
    n = len(m)
    d = Q(1)
    for c in range(n):
        p = next((i for i in range(c, n) if not m[i][c].iszero()), None)
        if p is None:
            return complex(0.0) if np.iscomplexobj(a) else 0.0
        if p != c:
            m[c], m[p] = m[p], m[c]
            d = Q(0) - d
        d = d * m[c][c]
        for i in range(c + 1, n):
            f = m[i][c] / m[c][c]
            m[i] = [x - f * y for x, y in zip(m[i], m[c])]
    return complex(float(d.re), float(d.im)) if np.iscomplexobj(a) else float(d.re)


# ---- writing ----
LINES = []


def save(case, role, arr):
    arr = np.ascontiguousarray(arr)
    arr = arr.astype("<c16" if np.iscomplexobj(arr) else "<f8")
    name = "%s_%s.npy" % (case, role)
    np.save(os.path.join(HERE, name), arr, allow_pickle=False)
    return name


def add(case, op, tol, arrays):
    files = [save(case, role, arr) for role, arr in arrays]
    LINES.append("%s %s %r %s" % (case, op, tol_of(tol), " ".join(files)))


def kind(a):
    return "c16" if np.iscomplexobj(a) else "f8"


def lu_solve(seed, a, k):
    n = a.shape[0]
    g = rng_for(seed + 1000)
    b = g.uniform(-1.0, 1.0, size=(n, k))
    if np.iscomplexobj(a):
        b = b + 1j * g.uniform(-1.0, 1.0, size=(n, k))
    x = np.linalg.solve(a, b)
    add("lu_solve_%s_%dx%d" % (kind(a), n, k), "lu_solve", 64 * n * EPS * cond(a), [("A", a), ("B", b), ("X", x)])


def det(a, exact=False):
    n = a.shape[0]
    d = exact_det(a) if exact else np.linalg.det(a)
    tol = 64 * n * EPS * (1.0 if exact else cond(a))
    add("det_%s_%d%s" % (kind(a), n, "_singular" if exact else ""), "det", tol, [("A", a), ("D", np.array([[d]]))])


def inv(a):
    n = a.shape[0]
    add("inv_%s_%d" % (kind(a), n), "inv", 64 * n * EPS * cond(a), [("A", a), ("X", np.linalg.inv(a))])


def svdvals(a, tag=""):
    m, n = a.shape
    s = np.linalg.svd(a, compute_uv=False)
    add("svdvals_%s_%dx%d%s" % (kind(a), m, n, tag), "svdvals", 32 * max(m, n) * EPS, [("A", a), ("S", s)])


def pinv(a, tag=""):
    m, n = a.shape
    p = max(m, n)
    s = np.linalg.svd(a, compute_uv=False)
    cut = p * EPS * s[0]
    kept = s[s > cut]
    # every dropped value must sit far below the cutoff so feng and numpy keep the same rank
    assert all(x < cut / 10 for x in s[s <= cut]), (tag, s, cut)
    assert all(x > cut * 1e6 for x in kept), (tag, s, cut)
    x = np.linalg.pinv(a, rtol=p * EPS)
    k = float(kept[0] / kept[-1])
    add("pinv_%s_%dx%d%s" % (kind(a), m, n, tag), "pinv", 64 * p * EPS * k * k, [("A", a), ("X", x)])


def cholesky(seed, n, complex_):
    b = cplx(seed, n, n) if complex_ else real(seed, n, n)
    a = b @ b.conj().T + n * np.eye(n)
    a = (a + a.conj().T) / 2  # exactly Hermitian
    add("cholesky_%s_%d" % (kind(a), n), "cholesky", 64 * n * EPS * cond(a), [("A", a), ("L", np.linalg.cholesky(a))])


def rref(a, tag=""):
    m, n = a.shape
    r = exact_rref(a)
    pivots = [next(j for j in range(n) if r[i, j] != 0) for i in range(m) if np.any(r[i] != 0)]
    k = cond(a[:, pivots])
    add("rref_%s_%dx%d%s" % (kind(a), m, n, tag), "rref", 64 * max(m, n) * EPS * k, [("A", a), ("R", r)])


def expm(a, tag=""):
    n = a.shape[0]
    nrm = max(1.0, float(np.linalg.norm(a, 1)))
    add("expm_%s_%d%s" % (kind(a), n, tag), "expm", 256 * n * EPS * nrm, [("A", a), ("E", scipy.linalg.expm(a))])


def main():
    for f in os.listdir(HERE):
        if f.endswith(".npy"):
            os.remove(os.path.join(HERE, f))

    lu_solve(11, well(real(11, 4, 4)), 2)
    lu_solve(12, real(12, 8, 8), 3)
    lu_solve(13, well(cplx(13, 5, 5)), 2)

    det(real(21, 4, 4))
    det(cplx(23, 6, 6))
    det(np.array([[1.0, 2, 3], [4, 5, 6], [7, 8, 9]]), exact=True)

    inv(well(real(31, 5, 5)))
    inv(real(32, 8, 8))
    inv(well(cplx(33, 4, 4)))

    svdvals(real(42, 8, 5))
    svdvals(real(43, 4, 7))
    svdvals(low_rank(44, 6, 6, 3), "_rank3")
    svdvals(cplx(45, 7, 4))
    svdvals(cplx(46, 3, 6))

    pinv(real(51, 6, 4))
    pinv(real(52, 3, 5))
    pinv(low_rank(53, 5, 5, 3), "_rank3")
    pinv(cplx(54, 5, 3))
    pinv(low_rank(55, 4, 6, 2, complex_=True), "_rank2")

    cholesky(61, 5, False)
    cholesky(62, 8, False)
    cholesky(63, 6, True)

    rref(np.array([[2.0, 1, 1], [1, 3, 2], [1, 0, 0]]))
    rref(low_rank(71, 4, 4, 2), "_rank2")
    rref(low_rank(72, 5, 3, 2), "_rank2")
    rref(low_rank(73, 3, 5, 3))
    rref(low_rank(74, 3, 4, 2, complex_=True), "_rank2")

    expm(real(81, 4, 4))
    expm(5 * real(82, 6, 6), "_norm5")
    expm(cplx(83, 4, 4))

    header = [
        "# S7-R6 oracle fixtures, written by tests/fixtures/s7/make_fixtures.py; do not edit by hand.",
        "# python %s, numpy %s, scipy %s" % (sys.version.split()[0], np.__version__, scipy.__version__),
        "# <case> <op> <tol> <files...>",
    ]
    with open(os.path.join(HERE, "manifest.txt"), "w", newline="\n") as f:
        f.write("\n".join(header + LINES) + "\n")


if __name__ == "__main__":
    main()
