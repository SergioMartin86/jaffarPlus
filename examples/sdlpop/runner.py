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
sourceScriptFilePath = segment + ".jaffar"
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
    
    # Replacing number in script
    newScript = ""
    for line in sourceScript.splitlines():
     if '"RNG Value"' in line: line = '    "RNG Value": ' + str(RNGSeed) + ','
     newScript += line + '\n'
     
    # Storing new script
    newScriptPath = "/tmp/tmp.jaffar"
    newScriptFile = open(newScriptPath, "w")
    newScriptFile.write(newScript)
    newScriptFile.close()
    
    # Running new seed
    print("Running Seed: " + str(RNGSeed) + "... ", end="", flush=True)
    rc=1
    stdoutFile = open("/tmp/jaffar.log", "a")
    result = subprocess.run(["jaffar", newScriptPath], stdout=stdoutFile, stderr=stdoutFile)
    stdoutFile.close()
    rc = result.returncode
    
    # Report results
    if (rc == 0):
      sourceSolutionFilePath = "/tmp/jaffar.best.sol"
      if os.path.isfile(sourceSolutionFilePath):
        num_lines = sum(1 for line in open(sourceSolutionFilePath))
        print("Success! Moves: " + str(num_lines))
        newSolutionPath = str(RNGSeed) + ".sol"
        shutil.copyfile(sourceSolutionFilePath, newSolutionPath)
      else:
        print("Could not load solution file: " + sourceSolutionFilePath)
        exit(-1)
        
    else:
      print("Fail!")
