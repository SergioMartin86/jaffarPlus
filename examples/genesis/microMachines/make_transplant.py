#!/usr/bin/env python3
"""Transplant a BizHawk Genesis waterbox savestate into a QuickerGPGX full state.

QuickerGPGX cannot load a BizHawk waterbox state directly, so we lift the dynamic
regions that define the running game out of the BizHawk state and overwrite them
into a known-good QuickerGPGX base state. The QuickerGPGX offsets were captured
from the emulator itself (GPGX_STATE_OFFSETS=1); the BizHawk offsets from a
region-correlation against the known-good race01a transplant (direct byte copies,
no endian swap):

  region     QuickerGPGX offset   size       BizHawk offset   what it fixes
  work_ram   0x11                 0x10000    0xCCA78          game logic (68k RAM)
  zram       0x10011              0x2000     0xDCA78          z80 RAM
  vdp_ctx    0x12066              0x10546    0x3B928          VIDEO: sat+vram+cram+vsram+reg+scalars
  m68k_regs  0x23455              0x5E       0xE0D20          68000 CPU registers

vdp_ctx is what the earlier (work_ram+zram+m68k only) transplant omitted, which
left the background/graphics garbled. Usage:
  make_transplant.py <bizhawk.state> <base.qgpgx.state> <out.state>
"""
import sys

# (name, qgpgx_offset, size, bizhawk_offset)
REGIONS = [
    ("work_ram", 0x11,     0x10000, 0xCCA78),
    ("zram",     0x10011,  0x2000,  0xDCA78),
    # vdp: sat+vram+cram+vsram+reg (the video DATA) but NOT the trailing transient scalars
    # (addr/code/status/fifo/dma_src at +0x10520); copying those transient DMA/FIFO fields into
    # QuickerGPGX triggers a bad DMA on load -> segfault. Scalars stay from the base state.
    ("vdp_ctx",  0x12066,  0x10520, 0x3B928),
    ("m68k_regs",0x23455,  0x5E,    0xE0D20),
]


def main():
    bz_path, base_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
    bz = open(bz_path, "rb").read()
    out = bytearray(open(base_path, "rb").read())
    for name, qoff, size, boff in REGIONS:
        assert boff + size <= len(bz), f"{name}: bizhawk region out of range"
        assert qoff + size <= len(out), f"{name}: qgpgx region out of range"
        out[qoff:qoff + size] = bz[boff:boff + size]
        print(f"  lifted {name}: bizhawk 0x{boff:X} ({size} B) -> qgpgx 0x{qoff:X}")
    open(out_path, "wb").write(out)
    print(f"wrote {out_path} ({len(out)} bytes)")


if __name__ == "__main__":
    main()
