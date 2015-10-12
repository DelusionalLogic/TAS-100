#!/bin/python2

import glob
import re

def fontCmp(x, y):
    if int(re.search("font_([0-9]*).xbm", x).group(1)) < int(re.search("font_([0-9]*).xbm", y).group(1)):
        return -1
    else:
        return 1

files = glob.glob("font_*.xbm")
print files.sort(cmp=fontCmp)
print files
fontMap = {}
with open("font.map", "r") as f:
    fontMap = {int(line.split(" -> ")[0]): line.split(" -> ")[1] for line in [x.rstrip() for x in f.readlines()]}
chars = [fontMap[charNum] for charNum in (int(re.search("font_([0-9]*).xbm", fName).group(1)) for fName in files)]

with open("font.inc", "w") as out:
    for fileName in files:
        with open(fileName, "r") as f:
            data = re.search(fileName.split(".")[0] + "_bits\[\] = \{ (.*) \};", f.read().replace('\n', '')).group(1).replace('   ', ' ').strip()
            out.write(data)
            out.write("\t\t/* " + chars[(int(re.search("font_([0-9]*).xbm", fileName).group(1)))] + " */")
            out.write("\n")

