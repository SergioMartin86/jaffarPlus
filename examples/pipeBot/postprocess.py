import argparse
import json

parser = argparse.ArgumentParser(prog='PipeBotPostProcessor', description='Takes a pipebot output and converts it into a NES output')
parser.add_argument('inputs')
parser.add_argument('initialRow')
parser.add_argument('initialCol')
args = parser.parse_args()

currentRow = int(args.initialRow)
currentCol = int(args.initialCol)
with open(args.inputs, 'r') as inputs_file: inputs = list(filter(None, inputs_file.read().split('\n')))

#print(currentRow)
#print(currentCol)

# First detecting replacements and sending them to the back
usedInputs = dict()
for input in inputs: usedInputs[input] = []
for idx, input in enumerate(inputs): usedInputs[input].append(idx)

currentReverseOrderIdx = 1
for k, v in usedInputs.items():
   idxSubList = v[:-1]
   for idx in idxSubList:
    inputs[idx] = inputs[-currentReverseOrderIdx]
    currentReverseOrderIdx = currentReverseOrderIdx + 1

nullInput  = '|..|........|'
pieceInput = '|..|.......A|'
framesUntilNextPiece = 0
usedInputs = set()
for input in inputs:
    inputList = input.replace('|', '').replace(' ', '').split(',')
    targetRow = int(inputList[0])
    targetCol = int(inputList[1])

    while (currentRow != targetRow) or (currentCol != targetCol):

        insertL = False
        insertR = False
        insertU = False
        insertD = False

        if (currentRow < targetRow): currentRow = currentRow + 1; insertD = True
        if (currentRow > targetRow): currentRow = currentRow - 1; insertU = True
        if (currentCol < targetCol): currentCol = currentCol + 1; insertR = True
        if (currentCol > targetCol): currentCol = currentCol - 1; insertL = True

        directionInput = list(nullInput)

        if (insertU == True): directionInput[4] = 'U'
        if (insertD == True): directionInput[5] = 'D'
        if (insertL == True): directionInput[6] = 'L'
        if (insertR == True): directionInput[7] = 'R'

        directionInputString = ''.join(directionInput)

        print(directionInputString)
        if (framesUntilNextPiece > 0): framesUntilNextPiece = framesUntilNextPiece - 1

        print(nullInput)
        if (framesUntilNextPiece > 0): framesUntilNextPiece = framesUntilNextPiece - 1

    # Wait until we can place a piece again
    while framesUntilNextPiece > 0:
        print(nullInput)
        framesUntilNextPiece = framesUntilNextPiece - 1

    # Now making piece placing input
    print(pieceInput)
    print(nullInput)

    # If the last piece didn't replace a piece, the frames to wait are 12 (minus one from the null input)
    framesUntilNextPiece = 11
    if (input in usedInputs): framesUntilNextPiece = 88

    # Adding piece to used piece set
    usedInputs.add(input)