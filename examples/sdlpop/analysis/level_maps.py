#!/usr/bin/env python3
import struct
from collections import deque

DAT = "/home/jaffar/jaffarPlus/examples/sdlpop/oldScripts/LEVELS.DAT"
data = open(DAT,"rb").read()
to, ts = struct.unpack_from("<IH", data, 0)
rc = struct.unpack_from("<H", data, to)[0]
entries = {}
for i in range(rc):
    rid, roff, rsz = struct.unpack_from("<HIH", data, to+2+i*8)
    entries[rid] = (roff, rsz)

def parse(n):
    roff, _ = entries[n+2000]
    b = data[roff+1:roff+1+2305]
    roomlinks = b[1952:1952+96]
    links = [(roomlinks[r*4],roomlinks[r*4+1],roomlinks[r*4+2],roomlinks[r*4+3]) for r in range(24)]
    p = 1952+96
    used = b[p]; p+=1
    p += 24+24+15
    start_room = b[p]; p+=1; start_pos=b[p]; p+=1
    start_dir = struct.unpack("b", b[p:p+1])[0]; p+=1; p+=4
    gtile = b[p:p+24]; p+=24
    gdir  = b[p:p+24]; p+=24
    gx    = b[p:p+24]; p+=24
    p+=24
    gskill= b[p:p+24]
    return links, used, start_room, start_pos, start_dir, gtile, gskill, gx

def build_map(links, start_room):
    # BFS assign grid coords; links are 1-based room ids, 0=none
    coord = {}
    if start_room < 1 or start_room > 24: start_room = 1
    coord[start_room] = (0,0)
    q = deque([start_room])
    seen = {start_room}
    delta = {0:(-1,0), 1:(1,0), 2:(0,-1), 3:(0,1)}  # L,R,U,D
    while q:
        r = q.popleft()
        x,y = coord[r]
        for di,(dx,dy) in delta.items():
            nb = links[r-1][di]
            if 1 <= nb <= 24 and nb not in seen:
                seen.add(nb); coord[nb]=(x+dx,y+dy); q.append(nb)
    return coord

for n in range(1,15):
    links, used, sr, sp, sd, gtile, gskill, gx = parse(n)
    coord = build_map(links, sr)
    if not coord:
        print("LEVEL %d: (no map)"%n); continue
    xs = [c[0] for c in coord.values()]; ys=[c[1] for c in coord.values()]
    minx,maxx=min(xs),max(xs); miny,maxy=min(ys),max(ys)
    grid = {}
    for r,(x,y) in coord.items(): grid[(x,y)]=r
    guards=[r+1 for r in range(24) if gtile[r]<30]
    print("="*70)
    print("LEVEL %d  start_room=%d start_pos=%d dir=%d  rooms_mapped=%d  guards_in_rooms=%s"%(
        n, sr, sp, sd, len(coord), guards))
    for y in range(miny,maxy+1):
        row=[]
        for x in range(minx,maxx+1):
            r=grid.get((x,y))
            if r is None: row.append("  . ")
            else:
                mark="*" if (r in guards) else (">" if r==sr else " ")
                row.append("%3d%s"%(r,mark))
        print("  "+"".join(row))
    # guard detail
    for r in guards:
        print("    room %d: guard skill=%d tile=%d"%(r, gskill[r-1], gtile[r-1]))
