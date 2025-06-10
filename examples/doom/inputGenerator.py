
#runValues = [ 0, 46, 47, 48, 49, 50 ]
#strafeValues = [ -50, -49, -48, 47, -46, 0, 46, 47, 48, 49, 50 ]
#turningValues = [ -64, -32, -27, -22, -16, -8, -7, -6, -5, -4, -3, -2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 22, 27, 32, 64]


runValues = [ 0, 46, 47, 48, 49, 50 ]
strafeValues = [ -50, -49, -48, 47, -46, 0, 46, 47, 48, 49, 50 ]
turningValues = [ -64, -32, -27, -22, -16, -8, -7, -6, -5, -4, -3, -2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 22, 27, 32, 64]

inputList = ''
for runValue in runValues:
 for strafeValue in strafeValues:
  for turningValue in turningValues:
   inputList += f'         "||{runValue: 4d},{strafeValue: 4d},{turningValue: 4d},   0,...|",\n'

print(inputList)
#print(len(inputList.splitlines()))