#!/usr/bin/env python3
import struct, sys, shutil

SRC="/home/jaffar/jaffarPlus/examples/sdlpop/oldScripts/LEVELS.DAT"
OUT="/tmp/chamber/chamber.dat"

data=bytearray(open(SRC,"rb").read())
to,ts=struct.unpack_from("<IH",data,0); rc=struct.unpack_from("<H",data,to)[0]
ent={}
for i in range(rc):
    rid,roff,rsz=struct.unpack_from("<HIH",data,to+2+i*8); ent[rid]=(roff,rsz)

# level 1 resource data start (after 1-byte checksum)
L1=ent[2001][0]+1

def setb(off,val): data[L1+off]=val&0xff

# ---- Room 1 corridor: row0 empty, row1 floor with a CLOSED gate, row2 wall ----
# fg index = room*30 + row*10 + col ; room1 -> base 0
GATE_COL = int(sys.argv[1]) if len(sys.argv)>1 else 6
GATE_BG  = int(sys.argv[2]) if len(sys.argv)>2 else 0   # gate openness modifier (0=closed)
row0=[0]*10                               # empty headroom
row1=[1]*10                               # floor corridor (level edges bound L/R)
row1[GATE_COL]=4                          # CLOSED gate
row2=[20]*10                              # solid wall floor below
fg_room1=row0+row1+row2
for p in range(30): setb(p, fg_room1[p])
# bg for room1 -> 0, except the gate cell's openness modifier
for p in range(30): setb(720+p, 0)
setb(720 + (10+GATE_COL), GATE_BG)   # row1 gate openness

# roomlinks[0] = (L,R,U,D) all 0 (isolated room)
for i in range(4): setb(1952+i, 0)
# used_rooms
setb(2048, 1)
# start_room / start_pos / start_dir : row1 col1 = tile 11, facing right(0)
setb(2112, 1)
setb(2113, 11)
setb(2114, 0)
# no guard in room 1
setb(2119+0, 0xFF)   # guards_tile[0] >= 30 -> no guard

shutil.copy(SRC, OUT)  # ensure file exists/size
open(OUT,"wb").write(bytes(data))
print("wrote", OUT, "gate at row1 col", GATE_COL)
