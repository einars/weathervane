#!/usr/bin/env python

import sys

with open(sys.argv[1], "rb") as f:
    data = f.read()

out = bytearray()
for g in range(0, len(data), 8):
    glyph = data[g:g+8]
    for x in range(8):
        row = 0
        for y in range(8):
            if glyph[y] & (1 << (7 - x)):
                row |= 1 << y
        out.append(row)

with open(sys.argv[2], "wb") as f:
    f.write(out)

