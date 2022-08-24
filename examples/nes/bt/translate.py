import argparse

parser = argparse.ArgumentParser(prog='FCEUX input -> bizhawk format translator')
parser.add_argument('source', metavar='source')
parser.add_argument('--twoPlayers', action='store_true', default=False)
args = parser.parse_args()

conversionDictionary = { 
 "A": ("A", 7),
 "B": ("B", 6),
 "S": ("s", 5),
 "T": ("S", 4),
 "U": ("U", 0),
 "D": ("D", 1),
 "L": ("L", 2),
 "R": ("R", 3)
 }

with open(args.source, 'r') as f: srcLines = f.readlines()
dstLines = []

for line in srcLines:
 dstLine = "|..|........|........|\n"
 dstLine = list(dstLine)
 offset = 4
 for move in [*line]:
   if move == '|': offset = 13
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

