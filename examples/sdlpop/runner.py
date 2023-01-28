#!/usr/bin/env python3

import os
import argparse
import subprocess
import random
import shutil

parser = argparse.ArgumentParser()
parser.add_argument('segment', help='Indicates the segment to run')
args = parser.parse_args()

# Getting segment ID
segment = args.segment

# Changing directory to the segment's
os.chdir(segment)

# Reading script file
sourceScriptFilePath = "script"
if os.path.isfile(sourceScriptFilePath):
    sourceScriptFile = open(sourceScriptFilePath, "r")
    sourceScript = sourceScriptFile.read()
    sourceScriptFile.close()
else:
    print("Could not load script file: " + file_path)
    exit(-1)

# Loop this forever
while True:

    # Producing new RNG number
    RNGSeed = random.randint(0,4294967295)
    solutionFilePath = "jaffar." + str(RNGSeed) + ".best.sol"
    newScriptPath = "script." + str(RNGSeed) + ".tmp"
    
    # Replacing number in script
    newScript = ""
    for line in sourceScript.splitlines():
     if '"RNG Value"' in line: line = '    "RNG Value": ' + str(RNGSeed) + ','
     if '"Frequency (s)"' in line: line = '    "Frequency (s)": 9999999.0,'
     if '"Best Solution Path"' in line: line = '    "Best Solution Path": "' + solutionFilePath + '",'
     newScript += line + '\n'
     
    # Storing new script
    newScriptFile = open(newScriptPath, "w")
    newScriptFile.write(newScript)
    newScriptFile.close()
    
    # Running new seed
    print("Running Seed: " + str(RNGSeed) + "... ", end="", flush=True)
    rc=1
    stdoutFile = open("jaffar.log", "a")
    result = subprocess.run(["jaffar", newScriptPath], stdout=stdoutFile, stderr=stdoutFile)
    stdoutFile.close()
    rc = result.returncode
    
    # Report results
    if (rc == 0):
      if os.path.isfile(solutionFilePath):
        num_lines = sum(1 for line in open(solutionFilePath))
        print("Success! Moves: " + str(num_lines))
      else:
        print("Could not load solution file: " + solutionFilePath)
        exit(-1)
        
    else:
      print("Fail!")
