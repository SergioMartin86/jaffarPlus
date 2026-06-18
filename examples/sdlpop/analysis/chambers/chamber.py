#!/usr/bin/env python3
# General SDLPoP chamber forge + search-config writer.
# Usage: chamber.py <scenario> [param]
import struct, sys, shutil, json

SRC="/home/jaffar/jaffarPlus/examples/sdlpop/oldScripts/LEVELS.DAT"
OUT="/tmp/chamber/chamber.dat"
TEMPLATE="/home/jaffar/jaffarPlus/examples/sdlpop/1200/script.jaffar"

T={'empty':0,'floor':1,'spike':2,'pillar':3,'gate':4,'doortop_floor':7,'potion':10,
   'loose':11,'doortop':12,'opener':15,'exitL':16,'exitR':17,'chomper':18,'wall':20}

def _open():
    data=bytearray(open(SRC,"rb").read())
    to,ts=struct.unpack_from("<IH",data,0); rc=struct.unpack_from("<H",data,to)[0]
    ent={}
    for i in range(rc):
        rid,roff,rsz=struct.unpack_from("<HIH",data,to+2+i*8); ent[rid]=(roff,rsz)
    return data, ent[2001][0]+1

def build_two_room_guard(skill=0, gcol=5, ceiling=False):
    """Room1=start corridor (no guard); Room1.right->Room2=corridor with a guard at row1 gcol.
       Kid runs right, crosses into Room2 -> enter_guard fires (room-entry).
       ceiling=True puts a solid row above the guard room (blocks jump-over)."""
    data,L1=_open()
    def setb(off,val): data[L1+off]=val&0xff
    floor_row=[T['floor']]*10
    top2=[T['wall']]*10 if ceiling else [T['empty']]*10
    r1=[T['empty']]*10+floor_row+[T['wall']]*10
    r2=top2+floor_row+[T['wall']]*10
    for p in range(30): setb(p, r1[p])          # room1 fg (idx0)
    for p in range(30): setb(30+p, r2[p])        # room2 fg (idx1)
    for p in range(60): setb(720+p, 0)           # bg rooms1-2 = 0
    # roomlinks: room1.right=2 ; room2.left=1   (link_type L,R,U,D at 1952 + room*4)
    setb(1952+1, 2)                              # room1 right -> 2
    setb(1956+0, 1)                              # room2 left  -> 1
    setb(2048, 2)                                # used_rooms
    setb(2112,1); setb(2113,11); setb(2114,0)    # start room1 row1 col1 facing right
    setb(2119+0,0xFF)                            # no guard room1
    # guard in room2 (index1): tile=row1*10+gcol
    setb(2119+1, 10+gcol); setb(2143+1,255); setb(2167+1,255)
    setb(2191+1,255); setb(2215+1,skill); setb(2239+1,255); setb(2263+1,2)
    shutil.copy(SRC,OUT); open(OUT,"wb").write(bytes(data))

def build_level(rows_fg, bg=None, start_pos=11, start_dir=0, guard=None):
    """rows_fg: list of 30 fg type-ints (room1, row-major 3x10). bg: dict pos->modifier."""
    data,L1=_open()
    def setb(off,val): data[L1+off]=val&0xff
    for p in range(30): setb(p, rows_fg[p])          # fg room1
    for p in range(30): setb(720+p, 0)               # bg room1 = 0
    if bg:
        for p,v in bg.items(): setb(720+p, v)
    for i in range(4): setb(1952+i,0)                # roomlinks[0]=0 (isolated)
    setb(2048,1)                                     # used_rooms
    setb(2112,1); setb(2113,start_pos); setb(2114,start_dir)
    if guard is None:
        setb(2119+0,0xFF)                            # no guard in room1
    else:
        setb(2119+0,guard['tile'])                   # guards_tile
        setb(2143+0,guard.get('dir',255)&0xff)       # guards_dir (-1 faces left)
        setb(2167+0,guard.get('x',255))              # guards_x
        setb(2191+0,guard.get('seq_lo',255))         # guards_seq_lo
        setb(2215+0,guard.get('skill',0))            # guards_skill
        setb(2239+0,guard.get('seq_hi',255))         # guards_seq_hi
        setb(2263+0,guard.get('color',2))            # guards_color (hi nibble = remembered hp)
    shutil.copy(SRC,OUT); open(OUT,"wb").write(bytes(data))

def write_search(win_conds=None, fail_conds=None, magnet_pos=255, magnet_y=None, max_steps=120, magnet_rooms=(1,), guard_hp_intensity=0.0):
    # magnet-only progress search: reward = Kid x (or y). best.sol = furthest-progress trajectory.
    s=json.load(open(TEMPLATE))
    e=s['Emulator Configuration']
    e['Initial State File']="/tmp/chamber/chamber.state"
    e['Levels File Path']="/tmp/chamber/chamber.dat"
    e['SDLPoP Root Path']="/home/jaffar/jaffarPlus/extern/quickerSDLPoP/SDLPoPData/"
    s['Driver Configuration']['End On First Win State']=False
    s['Driver Configuration']['Max Steps']=max_steps
    s['Engine Configuration']['State Database']['Max Size (Mb)']=4000
    s['Engine Configuration']['Hash Database']['Max Store Size (Mb)']=4000
    r=s['Runner Configuration']; r['Hash Step Tolerance']=0
    ALL=["|.|.....|","|.|L....|","|.|.R...|","|.|..U..|","|.|...D.|","|.|....S|",
         "|.|L.U..|","|.|.RU..|","|.|L..D.|","|.|.R.D.|","|.|L...S|","|.|.R..S|",
         "|.|..U.S|","|.|...DS|","|.|L.U.S|","|.|.RU.S|","|.|L..DS|","|.|.R.DS|"]
    r['Allowed Input Sets']=[{"Description":"full","Conditions":[],"Inputs":ALL,"Stop Input Evaluation":True}]
    r['Store Input History']={"Enabled":True,"Max Size":200}
    acts=[{"Type":"Set Kid Pos X Magnet","Intensity":1.0,"Position":magnet_pos,"Room":rm} for rm in magnet_rooms]
    if magnet_y is not None:
        acts+=[{"Type":"Set Kid Pos Y Magnet","Intensity":1.0,"Position":magnet_y,"Room":rm} for rm in magnet_rooms]
    if guard_hp_intensity:
        acts.append({"Type":"Set Guard HP Magnet","Intensity":guard_hp_intensity})
    rules=[{"Label":1,"Description":"magnet","Conditions":[],"Satisfies":[],"Actions":acts}]
    if win_conds:
        rules.append({"Label":2,"Description":"goal-reward","Conditions":win_conds,"Satisfies":[],
                      "Actions":[{"Type":"Set Kid Pos X Magnet","Intensity":1.0,"Position":magnet_pos,"Room":1}]})
    if fail_conds:
        for i,fc in enumerate(fail_conds):
            rules.append({"Label":100+i,"Description":"fail","Conditions":fc,"Satisfies":[],"Actions":[{"Type":"Trigger Fail"}]})
    s['Game Configuration']['Rules']=rules
    json.dump(s,open('/tmp/chamber/chamber.jaffar','w'),indent=2)

def corridor(modifications=None):
    """row0 empty, row1 floor, row2 wall. modifications: dict col->fgtype on row1."""
    fg=[T['empty']]*10 + [T['floor']]*10 + [T['wall']]*10
    if modifications:
        for c,t in modifications.items(): fg[10+c]=t
    return fg

scen=sys.argv[1]; param=int(sys.argv[2]) if len(sys.argv)>2 else None

if scen=='wall':
    build_level(corridor({6:T['wall']}))
    write_search()
elif scen=='gap':
    N=param or 2          # gap width in cols starting at col4
    mods={c:T['empty'] for c in range(4,4+N)}
    fg=corridor(mods)
    for c in range(4,4+N): fg[20+c]=T['empty']   # remove floor below gap too (fall=out)
    build_level(fg)
    write_search(fail_conds=[[{"Property":"Kid Pos Y","Op":">=","Value":160}]])  # fell below floor row
elif scen=='gate':
    bgv=param if param is not None else 0
    build_level(corridor({6:T['gate']}), bg={10+6:bgv})
    write_search()
elif scen=='guard':
    skill=param if param is not None else 0
    build_two_room_guard(skill=skill, gcol=5)
    write_search(fail_conds=[[{"Property":"Kid Current HP","Op":"==","Value":0}]],
                 max_steps=160, magnet_rooms=(1,2))
elif scen=='guardc':   # guard with a ceiling (blocks jump-over)
    skill=param if param is not None else 0
    build_two_room_guard(skill=skill, gcol=5, ceiling=True)
    write_search(fail_conds=[[{"Property":"Kid Current HP","Op":"==","Value":0}]],
                 max_steps=160, magnet_rooms=(1,2))
elif scen=='fall':
    # kid starts high on row0 floor (cols0-4), gap cols5-9 -> falls to row2 floor
    fg=[T['floor']]*5+[T['empty']]*5 + [T['empty']]*10 + [T['floor']]*10
    build_level(fg, start_pos=1, start_dir=0)   # start row0 col1
    write_search(magnet_pos=255, max_steps=120)
elif scen=='guardkill':
    skill=param if param is not None else 5
    build_two_room_guard(skill=skill, gcol=5)
    # reward approaching (small) + damaging the guard (big); see fastest-kill technique
    write_search(magnet_rooms=(), guard_hp_intensity=2000.0, max_steps=170)
elif scen=='xclip':
    # 2-room; room2 entry (cols0-3 on row1) is WALL -> kid crosses boundary into a wall
    import struct as _st
    data,L1=_open()
    def setb(off,val): data[L1+off]=val&0xff
    r1=[T['empty']]*10+[T['floor']]*10+[T['wall']]*10
    r2row1=[T['wall']]*4+[T['floor']]*6
    r2=[T['empty']]*10+r2row1+[T['wall']]*10
    for q in range(30): setb(q,r1[q])
    for q in range(30): setb(30+q,r2[q])
    for q in range(60): setb(720+q,0)
    setb(1952+1,2); setb(1956+0,1); setb(2048,2)
    setb(2112,1); setb(2113,11); setb(2114,0); setb(2119+0,0xFF); setb(2119+1,0xFF)
    shutil.copy(SRC,OUT); open(OUT,"wb").write(bytes(data))
    write_search(magnet_rooms=(1,2), max_steps=130)
elif scen=='bumpwarp':
    # Can a guard's strike knock the KID through a barrier at the room's right edge?
    # room1: floor corridor with a CLOSED gate at col8 (bg=0); room1.right->room2.
    # guard on the kid's LEFT (col3) so a strike pushes the kid RIGHT, toward the gate/edge.
    # win = Kid Room==2 (warped past the closed gate). fail = HP 0.
    data,L1=_open()
    def setb(off,val): data[L1+off]=val&0xff
    r1=[T['empty']]*10+[T['floor']]*10+[T['wall']]*10
    r1[10+8]=T['gate']                                  # closed gate at row1 col8
    r2=[T['empty']]*10+[T['floor']]*10+[T['wall']]*10
    for q in range(30): setb(q,r1[q])
    for q in range(30): setb(30+q,r2[q])
    for q in range(60): setb(720+q,0)                  # all bg 0 -> gate fully closed
    setb(1952+1,2); setb(1956+0,1); setb(2048,2)       # room1.right=2, room2.left=1
    setb(2112,1); setb(2113,55); setb(2114,0)          # kid room1 ~col5 facing right
    setb(2119+0,13); setb(2143+0,255); setb(2167+0,30) # guard room1 row1 col3, x~30
    setb(2191+0,255); setb(2215+0,5); setb(2239+0,255); setb(2263+0,2)
    setb(2119+1,0xFF)                                   # no guard room2
    shutil.copy(SRC,OUT); open(OUT,"wb").write(bytes(data))
    write_search(win_conds=[{"Property":"Kid Room","Op":"==","Value":2}],
                 fail_conds=[[{"Property":"Kid Current HP","Op":"==","Value":0}]],
                 magnet_rooms=(1,2), max_steps=200)
elif scen=='grabcorner':
    # Can a jump+grab snap the kid across/through a corner onto an unreachable ledge?
    # row0: floor cols0-2 (start ledge), empty cols3-9.
    # row1: wall col3 (a pillar/corner), empty elsewhere; row2 floor.
    # A grab-able lip on row0 at col5 (floor) separated from start by the gap+corner.
    fg=[T['floor']]*3+[T['empty']]*2+[T['floor']]*1+[T['empty']]*4   # row0
    fg+=[T['empty']]*3+[T['wall']]+[T['empty']]*6                    # row1 (wall corner col3)
    fg+=[T['floor']]*10                                              # row2 floor (catch)
    build_level(fg, start_pos=1, start_dir=0)          # start row0 col1 facing right
    write_search(magnet_pos=255, max_steps=130,
                 fail_conds=[[{"Property":"Kid Pos Y","Op":">=","Value":190}]])
else:
    print("unknown scenario",scen); sys.exit(1)
print("forged",scen,param,"-> chamber.dat + chamber.jaffar")
