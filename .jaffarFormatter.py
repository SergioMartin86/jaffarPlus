import json
import sys
from pprint import pprint

with open(sys.argv[1]) as f:
    d = json.load(f)
    if "Engine Configuration" in d:
      e = d["Engine Configuration"]
      if "State Database" in e:
       del d["Engine Configuration"]["State Database"]
       d["Engine Configuration"]["State Database"] = dict( { ("Max Size (Mb)", 1000) } )
    json_str = json.dumps(d, indent=4)

with open(sys.argv[1], "w") as f:
    f.write(json_str)

