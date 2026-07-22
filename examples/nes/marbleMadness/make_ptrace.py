#!/usr/bin/env python3
"""Generate a Progress Magnet trace for NES Marble Madness from a jaffar-player --dumpRam file.

Each output line: x y z vx vy airborne
 - x/y: world position (24-bit fixed point, hi*256 + mid + lo/256)
 - z:   height byte ($03C0)
 - vx/vy: true signed 16-bit sign-magnitude velocity (magnitude $03D8:$03D0 / $03E8:$03E0,
          direction flags $0410/$0418, flag 1 = negative)
 - airborne: 1 if $007B != 0 else 0

Usage: make_ptrace.py <dump.ram> <out.ptrace> [--slice LO,HI]  (LO/HI = frame range, HI exclusive)
"""
import sys

FRAME = 0x800

def main():
    ram, out = sys.argv[1], sys.argv[2]
    data = open(ram, 'rb').read()
    n = len(data) // FRAME
    lo, hi = 0, n
    if len(sys.argv) > 4 and sys.argv[3] == '--slice':
        lo, hi = (int(v) for v in sys.argv[4].split(','))
    def b(f, a): return data[f * FRAME + a]
    with open(out, 'w') as fh:
        for f in range(lo, min(hi, n)):
            x = b(f, 0x398) * 256 + b(f, 0x390) + b(f, 0x388) / 256
            y = b(f, 0x3B0) * 256 + b(f, 0x3A8) + b(f, 0x3A0) / 256
            z = b(f, 0x3C0)
            vx = (b(f, 0x3D8) * 256 + b(f, 0x3D0)) * (-1 if b(f, 0x410) else 1)
            vy = (b(f, 0x3E8) * 256 + b(f, 0x3E0)) * (-1 if b(f, 0x418) else 1)
            air = 1 if b(f, 0x7B) else 0
            fh.write(f"{x:.6f} {y:.6f} {z} {vx} {vy} {air}\n")
    print(f"wrote {min(hi, n) - lo} entries to {out}")

if __name__ == '__main__':
    main()
