#!/usr/bin/env python3
"""Generate the S5-R1 NPY fixtures with numpy. Run from the repo root:
    python3 tests/fixtures/s5/make_npy.py
The values are fixed so tests/cases/s5_r1.hpp can compare against them."""
import os
import numpy as np

here = os.path.dirname(os.path.abspath(__file__))
base = np.array([[1.5, -2.25, 3.0], [4.125, 5.0, -6.5]])


def save(name, arr, **kw):
    np.save(os.path.join(here, name), arr, **kw)


save("f8_le.npy", base.astype("<f8"))
save("f8_be.npy", base.astype(">f8"))
save("f4_le.npy", base.astype("<f4"))
save("i4_be.npy", np.array([[1, -2, 3], [70000, -80000, 2147483647]], dtype=">i4"))
save("c16_le.npy", np.array([[1 + 2j, -3.5 + 0.25j], [0 - 1j, 7 + 0j]], dtype="<c16"))
save("c16_be.npy", np.array([[1 + 2j, -3.5 + 0.25j], [0 - 1j, 7 + 0j]], dtype=">c16"))
save("u1.npy", np.array([[0, 1, 255], [128, 7, 42]], dtype="|u1"))
save("fortran_2x3.npy", np.asfortranarray(base.astype("<f8")))
save("one_d.npy", np.array([10.0, 20.0, 30.0], dtype="<f8"))
save("three_d.npy", np.arange(8, dtype="<f8").reshape(2, 2, 2))
save("object.npy", np.array([[1, "a"], [None, 2.0]], dtype=object), allow_pickle=True)
save("structured.npy", np.zeros((2, 3), dtype=[("x", "<f8"), ("y", "<i4")]))

# S5-T7: every listed dtype in both byte orders, the rejected bool and f2 dtypes, and v2.0 / v3.0 headers.
i1 = [[-128, -1, 0], [1, 64, 127]]
i2 = [[-32768, -2, 0], [1, 300, 32767]]
i4 = [[1, -2, 3], [70000, -80000, 2147483647]]
i8 = [[-9223372036854775808, -1, 0], [1, 1099511627776, 9223372036854775807]]
u2 = [[0, 1, 65535], [256, 4660, 65280]]
u4 = [[0, 1, 4294967295], [65536, 305419896, 4278190080]]
u8 = [[0, 1, 18446744073709551615], [4294967296, 81985529216486895, 18374686479671623680]]
cx = [[1 + 2j, -3.5 + 0.25j], [0 - 1j, 7 + 0j]]
save("i1.npy", np.array(i1, dtype="|i1"))
save("i2_le.npy", np.array(i2, dtype="<i2"))
save("i2_be.npy", np.array(i2, dtype=">i2"))
save("i4_le.npy", np.array(i4, dtype="<i4"))
save("i8_le.npy", np.array(i8, dtype="<i8"))
save("i8_be.npy", np.array(i8, dtype=">i8"))
save("u2_le.npy", np.array(u2, dtype="<u2"))
save("u2_be.npy", np.array(u2, dtype=">u2"))
save("u4_le.npy", np.array(u4, dtype="<u4"))
save("u4_be.npy", np.array(u4, dtype=">u4"))
save("u8_le.npy", np.array(u8, dtype="<u8"))
save("u8_be.npy", np.array(u8, dtype=">u8"))
save("f4_be.npy", base.astype(">f4"))
save("c8_le.npy", np.array(cx, dtype="<c8"))
save("c8_be.npy", np.array(cx, dtype=">c8"))
save("bool.npy", np.array([[True, False, True], [False, True, False]], dtype="|b1"))
save("f2.npy", base.astype("<f2"))
for version in ((2, 0), (3, 0)):
    with open(os.path.join(here, "f8_v%d.npy" % version[0]), "wb") as f:
        np.lib.format.write_array(f, base.astype("<f8"), version=version)
