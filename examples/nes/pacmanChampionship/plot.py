import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import numpy as np

file_path = 'twistedEye.score'
with open(file_path, 'r') as file:
    twistedEyeLines = file.readlines()

file_path = 'warHippy.score'
with open(file_path, 'r') as file:
    warHippyLines = file.readlines()    

file_path = 'jaffarv1.score'
with open(file_path, 'r') as file:
    jaffarV1Lines = file.readlines()    

file_path = 'jaffarv2.score'
with open(file_path, 'r') as file:
    jaffarV2Lines = file.readlines()        

twistedEyeScoreHistory = []
x = []
curVal = 0
for line in twistedEyeLines:
    twistedEyeScoreHistory.append(int(line))
    x.append(curVal)
    curVal += 1

warHippyScoreHistory = []
for line in warHippyLines:
    warHippyScoreHistory.append(int(line))

jaffarV1ScoreHistory = []
for line in jaffarV1Lines:
    jaffarV1ScoreHistory.append(int(line))

jaffarV2ScoreHistory = []
for line in jaffarV2Lines:
    jaffarV2ScoreHistory.append(int(line))

plt.plot(x, twistedEyeScoreHistory)
plt.plot(x, warHippyScoreHistory)
plt.plot(x, jaffarV1ScoreHistory)
plt.plot(x, jaffarV2ScoreHistory)

plt.legend(['TwistedEye', 'WarHippy', 'JaffarPlus v1', 'JaffarPlus v2'])
plt.xlabel("Frame #")
plt.ylabel("Score")
plt.grid()
plt.xlim(0, 18338)
plt.ylim(0, 751700)
plt.show()