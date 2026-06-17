import json, os, copy, subprocess, sys
sys.path.insert(0,'/tmp')
from migrate import migrate_script, conv_line
from migrate_old import migrate_old_schema
PLAYER='/home/jaffar/jaffarPlus/build-sdlpop/jaffar-player'
ROOT='../../../../extern/quickerSDLPoP/SDLPoPData/'

def sol_ok(path):
    for ln in open(path):
        ln=ln.rstrip('\n')
        if ln=='':continue
        if ln.startswith('|') and len(ln)==9: continue
        if len(ln)==7 and all(ln[i] in '.CALRUDS' for i in range(7)): continue
        if 1<=len(ln)<=5 and all(c in '.CALRUDS' for c in ln): continue
        return False
    return True

def process(folder):
    r={'f':folder}
    if not all(os.path.exists(f'{folder}/{x}') for x in ('script.jaffar.old','state','solution')):
        r['st']='SKIP missing files'; return r
    if not sol_ok(f'{folder}/solution'):
        r['st']='SKIP odd-solution-format'; return r
    old=json.load(open(f'{folder}/script.jaffar.old'))
    keys=set(old.keys())
    if 'Engine Configuration' in keys: new=migrate_script(old,'state',ROOT); r['sch']='new'
    elif 'Jaffar Configuration' in keys: new=migrate_old_schema(old,ROOT); r['sch']='old'
    else: r['st']='SKIP unknown-schema'; return r
    json.dump(new, open(f'{folder}/script.jaffar','w'), indent=2)
    out=[conv_line(l) for l in open(f'{folder}/solution')]; out=[x for x in out if x]
    open(f'{folder}/solution.sol','w').write('\n'.join(out)+'\n'); r['n']=len(out)
    val=copy.deepcopy(new); val['Game Configuration']['Frame Rate']=100000.0
    json.dump(val, open(f'{folder}/.val.jaffar','w'))
    try:
        cp=subprocess.run([PLAYER,'.val.jaffar','solution.sol','--reproduce','--disableRender','--exitOnEnd','--unattended','--printFinalState'],
            cwd=folder, capture_output=True, text=True, timeout=180, env={**os.environ,'OMP_NUM_THREADS':'1'})
        o=cp.stdout+cp.stderr
        ft=[l.split(':')[-1].strip() for l in o.splitlines() if 'Final State Type:' in l]
        fs=[l.split(':')[-1].strip() for l in o.splitlines() if 'Final Step:' in l]
        err=any(k in o for k in ('THROW','error:','terminate','Could not'))
        r['final']=ft[-1] if ft else '?'; r['fstep']=fs[-1] if fs else '?'
        if cp.returncode!=0 or err: r['st']='ERROR'
        elif r['fstep']==str(r['n']): r['st']='CLEAN'+('/WIN' if r['final']=='Win' else '')
        else: r['st']='PARTIAL?'
    except subprocess.TimeoutExpired: r['st']='TIMEOUT'
    finally:
        if os.path.exists(f'{folder}/.val.jaffar'): os.remove(f'{folder}/.val.jaffar')
    return r

for f in sorted(d for d in os.listdir('.') if d.isdigit() and len(d)==4):
    r=process(f)
    print(f"{r['f']}  {r.get('sch','-'):3} {r.get('st',''):16} inputs={r.get('n','-'):>5} finalStep={r.get('fstep','-'):>5} final={r.get('final','-')}")
