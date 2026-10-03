#!/usr/bin/env python3
# S5-R5: writes the seed corpora under tests/fuzz/corpus/<target>/ (run from the repo root). NPY seeds are copies of
# tests/fixtures/s5/*.npy plus a truncated one; the others are built here. Valid and near-valid inputs, all small.
import os
import shutil
import struct

ROOT = os.path.join("tests", "fuzz", "corpus")


def put(target, name, data):
    d = os.path.join(ROOT, target)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, name), "wb") as f:
        f.write(data)


# NPY
for fx in ["f8_le", "f8_be", "c16_le", "i4_be", "fortran_2x3", "one_d", "u1", "structured"]:
    os.makedirs(os.path.join(ROOT, "npy"), exist_ok=True)
    shutil.copy(os.path.join("tests", "fixtures", "s5", fx + ".npy"), os.path.join(ROOT, "npy", fx + ".npy"))
with open(os.path.join("tests", "fixtures", "s5", "f8_le.npy"), "rb") as f:
    put("npy", "f8_le_truncated.npy", f.read()[:-5])


# BMP: 14-byte file header + 40-byte info header, BI_RGB.
def bmp(width, height, bpp, trailing=b""):
    rows = abs(height)
    stride = (width * bpp + 31) // 32 * 4
    pixels = bytearray()
    for r in range(rows):
        row = bytearray()
        for c in range(width):
            px = bytes([(r * 40 + c) & 255, (c * 60) & 255, (r * 90 + 7) & 255])
            row += px + (b"\xff" if bpp == 32 else b"")
        row += b"\0" * (stride - len(row))
        pixels += row
    offset = 54
    header = b"BM" + struct.pack("<IHHI", offset + len(pixels) + len(trailing), 0, 0, offset)
    info = struct.pack("<IiiHHIIiiII", 40, width, height, 1, bpp, 0, len(pixels), 2835, 2835, 0, 0)
    return header + info + bytes(pixels) + trailing


put("bmp", "rgb24_bottom_up_3x2.bmp", bmp(3, 2, 24))
put("bmp", "rgb24_top_down_3x2.bmp", bmp(3, -2, 24))
put("bmp", "rgb32_2x2.bmp", bmp(2, 2, 32))
put("bmp", "rgb24_trailing_1x1.bmp", bmp(1, 1, 24, b"xyz"))
put("bmp", "rgb24_truncated.bmp", bmp(3, 2, 24)[:-4])

# Native binary: two size_t counts (native little-endian, 8 bytes) then the elements.
put("binary", "double_2x3.bin", struct.pack("<QQ", 2, 3) + struct.pack("<6d", 1, 2, 3, 4, 5, 6))
put("binary", "int32_2x2.bin", struct.pack("<QQ", 2, 2) + struct.pack("<4i", 1, -2, 3, -4))
put("binary", "empty_0x3.bin", struct.pack("<QQ", 0, 3))
put("binary", "double_2x3_short.bin", struct.pack("<QQ", 2, 3) + struct.pack("<5d", 1, 2, 3, 4, 5))

# Text
put("text", "real_2x3.txt", b"1 2 3\n4 5 6\n")
put("text", "int_tabs_crlf.txt", b"1\t-2\r\n3\t4\r\n")
put("text", "complex_2x2.txt", b"(1,2) (3,-4)\n(0.5,0) 7\n")
put("text", "float_exp.txt", b"1.5e3 -2.25e-2\n+3 0x10\n")
put("text", "ragged.txt", b"1 2\n3\n")
put("text", "unclosed.txt", b"(1,2 3\n")
