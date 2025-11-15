import argparse
import json

parser = argparse.ArgumentParser(prog='PipeBotPostProcessor', description='Takes a pipebot output and converts it into a NES output')
parser.add_argument('script')
parser.add_argument('inputs')
args = parser.parse_args()


with open(args.script, 'r') as script_file: config = json.load(script_file)
currentRow = config["Game Configuration"]["Distance Limiter"]["Initial Row"]
currentCol = config["Game Configuration"]["Distance Limiter"]["Initial Col"]

args = parser.parse_args()
with open(args.inputs, 'r') as inputs_file: inputs = list(filter(None, inputs_file.read().split('\n')))

for input in inputs:
    print(input)

