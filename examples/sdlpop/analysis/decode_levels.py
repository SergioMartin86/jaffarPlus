#!/usr/bin/env python3
import struct, sys

DAT = "/home/jaffar/jaffarPlus/examples/sdlpop/oldScripts/LEVELS.DAT"
data = open(DAT,"rb").read()

table_offset, table_size = struct.unpack_from("<IH", data, 0)
res_count = struct.unpack_from("<H", data, table_offset)[0]
entries = {}
for i in range(res_count):
    off = table_offset + 2 + i*8
    rid, roff, rsize = struct.unpack_from("<HIH", data, off)
    entries[rid] = (roff, rsize)

def get_level(n):
    rid = n + 2000
    roff, rsize = entries[rid]
    # data starts after 1 checksum byte; size field is the data size
    blob = data[roff+1:roff+1+2305]
    return blob, rsize

def parse(blob):
    # offsets per level_type
    fg = blob[0:720]
    bg = blob[720:1440]
    doorlinks1 = blob[1440:1696]
    doorlinks2 = blob[1696:1952]
    roomlinks = blob[1952:1952+96]  # 24*4
    p = 1952+96
    used_rooms = blob[p]; p+=1
    roomxs = blob[p:p+24]; p+=24
    roomys = blob[p:p+24]; p+=24
    p+=15  # fill_1
    start_room = blob[p]; p+=1
    start_pos = blob[p]; p+=1
    start_dir = struct.unpack("b", blob[p:p+1])[0]; p+=1
    p+=4  # fill_2
    guards_tile = blob[p:p+24]; p+=24
    guards_dir  = blob[p:p+24]; p+=24
    guards_x    = blob[p:p+24]; p+=24
    guards_seq_lo=blob[p:p+24]; p+=24
    guards_skill= blob[p:p+24]; p+=24
    guards_seq_hi=blob[p:p+24]; p+=24
    guards_color= blob[p:p+24]; p+=24
    links = [(roomlinks[r*4],roomlinks[r*4+1],roomlinks[r*4+2],roomlinks[r*4+3]) for r in range(24)]
    return dict(used_rooms=used_rooms, links=links, roomxs=roomxs, roomys=roomys,
                start_room=start_room, start_pos=start_pos, start_dir=start_dir,
                guards_tile=guards_tile, guards_skill=guards_skill, guards_x=guards_x,
                guards_dir=guards_dir, fg=fg)

for n in range(1,15):
    blob,skip = get_level(n)
    L = parse(blob)
    ur = L['used_rooms']
    print("="*60)
    print("LEVEL %d  used_rooms=%d start_room=%d start_pos=%d start_dir=%d (skip=%d)"%(
        n, ur, L['start_room'], L['start_pos'], L['start_dir'], skip))
    print("  room: L  R  U  D    (links, 0=none)")
    for r in range(1, 25):
        l,rr,u,d = L['links'][r-1]
        g=""
        gt = L['guards_tile'][r-1]
        if gt < 30:
            g = "  GUARD tile=%d skill=%d x=%d dir=%d"%(gt, L['guards_skill'][r-1], L['guards_x'][r-1], struct.unpack("b",bytes([L['guards_dir'][r-1]]))[0])
        print("   %2d:  %2d %2d %2d %2d%s"%(r,l,rr,u,d,g))
