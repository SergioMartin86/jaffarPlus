import argparse

parser = argparse.ArgumentParser(prog='bizhawk -> quicknes transplanter')
parser.add_argument('--base',  required=True)
parser.add_argument('--source',  required=True)
parser.add_argument('--dest',  required=True)
args = parser.parse_args()

sourceFile = open(args.source, 'rb')
baseFile = open(args.base, 'rb');
destFile = open(args.dest, 'wb');

baseArray = bytearray()
while (byte := baseFile.read(1)):
    baseArray += byte
    
sourceArray = bytearray()
while (byte := sourceFile.read(1)):
    sourceArray += byte
    
print(len(sourceArray))
print(len(baseArray))

# Copying NES RAM
srcStartIdx = 0x003D
baseStartIdx = 0x00D9
i = 0
while i < 0x0800:
  baseArray[baseStartIdx + i] = sourceArray[srcStartIdx + i]
  i = i+1 
  
# Copying NES CHR
srcStartIdx = 0x108A
baseStartIdx = 0x11F1
i = 0
while i < 0x2000:
  baseArray[baseStartIdx + i] = sourceArray[srcStartIdx + i]
  i = i+1
  
# Copying NES Nametables
srcStartIdx = 0x0841
baseStartIdx = 0x09E9
i = 0
while i < 0x0800:
  baseArray[baseStartIdx + i] = sourceArray[srcStartIdx + i]
  i = i+1
  
# Copying NES OAM
srcStartIdx = 0x3133
baseStartIdx = 0x08E1
i = 0
while i < 0x0100:
  baseArray[baseStartIdx + i] = sourceArray[srcStartIdx + i]
  i = i+1
  
destFile.write(baseArray)
