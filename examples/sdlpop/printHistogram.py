import os

histogram = dict()

for r, d, f in os.walk('.'):
 for file in f: 
  if file == 'jaffar.best.sol':
   pDirs = r.split('/')
   segment = pDirs[1]

   filePath = os.path.join(r, file)
   lineCount = -1 
   with open(filePath, 'r') as fp:
    for line in fp:
     if line.strip():
      lineCount += 1

   if not segment in histogram: histogram[segment] = dict()
   if not lineCount in histogram[segment]: histogram[segment][lineCount] = 1
   else: histogram[segment][lineCount] = histogram[segment][lineCount] + 1

for s in sorted(histogram):
  print(s)
  for l in sorted(histogram[s], reverse=True):
      print("  + " + str(l) + ": " + str(histogram[s][l]))
