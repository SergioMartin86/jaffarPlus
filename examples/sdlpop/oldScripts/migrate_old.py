import json, copy, os
# Reuse the generic PoP input sets / frameskip from the already-migrated 0100 newer-schema script.
_ref = json.load(open('0100/script.jaffar'))['Runner Configuration']

PROP_MAP={'Kid Position Y':'Kid Pos Y','Kid Position X':'Kid Pos X',
          'Guard Position X':'Guard Pos X Raw','Guard Position Y':'Guard Pos Y Raw',
          'Exit Door Timer':'Exit Room Timer'}
ACT_MAP={'Set Guard Horizontal Magnet':'Set Guard Pos X Magnet',
         'Set Guard Vertical Magnet':'Set Guard Pos Y Magnet'}
def _fix(obj):
    if isinstance(obj, dict):
        pr=str(obj.get('Property',''))
        if ('Room' in obj and 'Tile' in obj) and (pr.endswith('[') or pr.startswith('Tile ')):
            base='Foreground Element' if ('FG' in pr or pr.startswith('Foreground')) else 'Background Element'
            obj=dict(obj); obj['Property']=f"{base}[{obj['Room']}][{obj['Tile']}]"
            for k in ('Room','Tile','Level','Comment'): obj.pop(k,None)
        o={}
        for k,v in obj.items():
            if k=='Center': k='Position'
            o[k]=_fix(v)
        if 'Property' in o: o['Property']=PROP_MAP.get(o['Property'], o['Property'])
        if 'Type' in o: o['Type']=ACT_MAP.get(o['Type'], o['Type'])
        return o
    if isinstance(obj, list): return [_fix(x) for x in obj]
    return obj

def migrate_old_schema(old, root, version='1.0'):
    em=old['Emulator Configuration']; gc=old['Game Configuration']; jc=old['Jaffar Configuration']
    sc=jc.get('State Configuration',{}); sdb=jc.get('State Database',{}); hdb=jc.get('Hash Database',{})
    sr=jc.get('Save Intermediate Results',{})
    looseId = em.get('Loose Tile Sound Id', 0)
    new={
      'Driver Configuration':{
        'End On First Win State': True, 'Max Steps': 0,
        'Save Intermediate Results': sr if sr else {'Enabled':True,'Frequency (s)':1.0,
            'Best Solution Path':'/tmp/jaffar.best.sol','Worst Solution Path':'/tmp/jaffar.worst.sol',
            'Best State Path':'/tmp/jaffar.best.state','Worst State Path':'/tmp/jaffar.worst.state'},
      },
      'Engine Configuration':{
        'Hash Database':{'Max Store Count': hdb.get('Database Count',2),
                         'Max Store Size (Mb)': hdb.get('Max Size (Mb)',10000),'Enabled':True},
        'State Database':{'Max Size (Mb)': 4000,
                          'Differential Compression':{'Enabled': sc.get('XDelta3 Enabled',False),
                              'Buffer Size':1024,'Use Zlib Compression': sc.get('XDelta3 Use Zlib Compression',False)}},
        'NUMA Domains Per Group':1,
      },
      'Emulator Configuration':{
        'Emulator Name':'QuickerSDLPoP','Initial State File':'state','Disabled State Properties':[],
        'SDLPoP Root Path': root,'Levels File Path':'','Game Version': version,
        'Override RNG Enabled': bool(em.get('Override RNG Value',True)),
        'Override RNG Value': int(em.get('RNG Value',0)),
        'Override Loose Tile Sound Enabled': looseId!=0,
        'Override Loose Tile Sound Value': looseId,
        'Initialize Copy Protection': bool(em.get('Initialize Copyprot',False)),
      },
      'Runner Configuration': copy.deepcopy(_ref),
      'Game Configuration':{
        'Game Name':'SDLPoP / Prince of Persia','Frame Rate':24.0,
        'Print Properties':['Foreground Element[1][27]','Background Element[1][27]','Foreground Element[9][14]','Background Element[9][14]'],
        'Hash Properties':['Foreground Element[1][27]','Background Element[1][27]','Foreground Element[9][14]','Background Element[9][14]'],
        'Rules': _fix(old['Rules']),
        'Bypass Emulator State': False,
      },
    }
    # Carry over the old timer tolerance into the runner's hash-step tolerance, and move count
    new['Runner Configuration']['Hash Step Tolerance']=gc.get('Timer Tolerance', new['Runner Configuration'].get('Hash Step Tolerance',1))
    new['Runner Configuration']['Store Input History']={'Enabled':True,'Max Size': sc.get('Max Move Count',1800)}
    return new

if __name__=='__main__':
    import sys
    folder=sys.argv[1]; root='../../../../extern/quickerSDLPoP/SDLPoPData/'
    old=json.load(open(f'{folder}/script.jaffar.old'))
    json.dump(migrate_old_schema(old, root), open(f'{folder}/script.jaffar','w'), indent=2)
    print(f"{folder}: migrated (older schema)")
