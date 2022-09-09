import argparse

parser = argparse.ArgumentParser(prog='FCEUX input -> bizhawk format translator')
parser.add_argument('source', metavar='source')
parser.add_argument('--twoPlayers', action='store_true', default=False)
args = parser.parse_args()

conversionDictionary = { 
 "A": ("A", 0),
 "B": ("B", 1),
 "S": ("s", 2),
 "T": ("S", 3),
 "U": ("U", 4),
 "D": ("D", 5),
 "L": ("L", 6),
 "R": ("R", 7)
 }

with open(args.source, 'r') as f: srcLines = f.readlines()
dstLines = []

for line in srcLines:
 dstLine = "|..|........|"
 offset = 4
 dstLine = list(dstLine)
 for move in [*line]:
   if move in conversionDictionary:
     pos = conversionDictionary[move][1] + offset
     newMove = conversionDictionary[move][0]
     dstLine[pos] = newMove
 dstLines.append("".join(dstLine))

if (not args.twoPlayers):
 srcLines = dstLines
 dstLines = []
 for line in srcLines:
  dstLines.append(line[0:13] + '\n')

with open(args.source + '.translated', 'w') as f: f.writelines(dstLines)

