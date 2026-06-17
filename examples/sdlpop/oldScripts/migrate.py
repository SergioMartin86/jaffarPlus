import json, os, copy

def conv_line(ln):
    ln = ln.rstrip('\n')
    if ln == '': return None
    if ln.startswith('|'):                       # already new pipe format
        return ln
    if len(ln) == 7 and all(ln[i] in '.CALRUDS' for i in range(7)):   # old positional [C][A][L][R][U][D][S]
        C,A,L,R,U,D,S = (ln[i] for i in range(7))
        restart = 'r' if (C=='C' or A=='A') else '.'                  # Ctrl+A = restart level
        return "|%s|%s%s%s%s%s|" % (restart,
            'L' if L=='L' else '.', 'R' if R=='R' else '.', 'U' if U=='U' else '.',
            'D' if D=='D' else '.', 'S' if S=='S' else '.')
    if 1 <= len(ln) <= 5 and all(ch in '.CALRUDS' for ch in ln):      # compact button-set ("LU","SR","CA",".")
        restart='r' if ('C' in ln or 'A' in ln) else '.'             # Ctrl+A = restart
        return "|%s|%s%s%s%s%s|" % (restart,
            'L' if 'L' in ln else '.', 'R' if 'R' in ln else '.', 'U' if 'U' in ln else '.',
            'D' if 'D' in ln else '.', 'S' if 'S' in ln else '.')
    raise ValueError("unexpected solution line: %r" % ln)

def convert_solution(src, dst):
    out=[conv_line(l) for l in open(src)]
    out=[x for x in out if x is not None]
    open(dst,'w').write('\n'.join(out)+'\n')
    return len(out)

def migrate_script(old, state_file, root_path):   # newer (0100-style) schema
    c=copy.deepcopy(old); eng=c['Engine Configuration']; sd=eng['State Database']
    old_sizes=sd.get('Max Size per NUMA Domain (Mb)', [1000]); comp=sd.get('Compression', {})
    eng['State Database']={'Max Size (Mb)': sum(old_sizes) if isinstance(old_sizes,list) else old_sizes,
        'Differential Compression': {'Enabled': comp.get('Use Differential Compression', False),
            'Buffer Size': 1024, 'Use Zlib Compression': comp.get('Use Zlib Compression', False)}}
    eng['Hash Database']['Enabled']=True; eng['NUMA Domains Per Group']=1
    em=c['Emulator Configuration']; em['Initial State File']=state_file; em['SDLPoP Root Path']=root_path
    em.setdefault('Disabled State Properties', [])
    r=c['Runner Configuration']; r.setdefault('Bypass Hash Calculation', False)
    r['Frameskip']={'Rate': r.get('Frameskip Rate',0), 'Use Custom Input': False, 'Custom Input': ''}
    c['Game Configuration'].setdefault('Bypass Emulator State', False)
    return c
