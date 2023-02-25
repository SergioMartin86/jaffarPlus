import json
import os
import math
import matplotlib.pyplot as plt
   
with open('graph.json', 'r') as f:
 graph = json.load(f)

 
#for x in graph['Graph']:
#  for edge in x['Edges']:
    
    

import matplotlib.pyplot as plt
 
figure, axes = plt.subplots()
axes.set_aspect( 1 )
axes.set_xlim([-0.1, 1.05])
axes.set_ylim([-0.1, 1.4])


childSets = []

for x in graph['Graph']:
  currentChildSet = set()
  for e in x['Edges']:
    currentChildSet.add((e[0], e[1]))
  childSets.append(currentChildSet)

axes.add_artist(plt.Circle((1.0, 0.0), 0.005 ))

parentXMap = dict()
for i, x in enumerate(graph['Graph']):
  childXMap = dict()
  edgeCount = len(x['Edges'])
  curChild = 0
  for j, e in enumerate(childSets[i]):
    cpy = 1.2 - i / 15
    cpx = curChild / len(childSets[i])   
    ppy = 1.2 - (i-1) / 15
    
    curChild += 1 
    
    axes.add_artist(plt.Circle((cpx, cpy), radius=0.005 ))
    
    if e[0] in parentXMap:
        ppx = parentXMap[e[0]]
        axes.add_artist(plt.Line2D([ppx,cpx], [ppy,cpy], linewidth=0.1))
    
    childXMap[e[1]] = cpx
  parentXMap = childXMap
        
plt.show()