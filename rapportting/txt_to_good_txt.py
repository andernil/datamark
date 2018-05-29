# -*- coding: utf-8 -*-
"""
Created on Fri Mar 30 11:53:23 2018

@author: anders
"""

import fileinput
import os
import sys
import re

program = ['ammp', 'applu', 'apsi', 'art110','art470', 'bzip2_graphic', 'bzip2_program',
           'bzip2_source', 'galgel', 'swim', 'twolf', 'wupwise', 'user']

def main():
    infile = open('sim', 'r', encoding="UTF-8")
    outfile = open('sim_good', 'w')
    for line in infile:
        unn = []
        newline = ''
        count = 0
        if 'DCPT' in re.findall(r"[\w']+", line):
            newline = 'DCPT \n'
        if 'RPT' in re.findall(r"[\w']+", line):
            newline = 'RPT \n'
        if 'seq_miss' in re.findall(r"[\w']+", line):
            newline = 'seq_miss \n'
        if 'seq_acc' in re.findall(r"[\w']+", line):
            newline = 'seq_acc \n'
        if 'noalg' in re.findall(r"[\w']+", line):
            newline = 'none \n'
        for i in range(0, len(program)):        
            if program[i] in re.findall(r"[\w']+", line):
                print(line)
                newline = re.split('   ', line)
                print(newline)
                length = len(newline)
#                for j in range(0, length):
#                    if newline[j] == '':
#                        count += 1
#                        if count > 2:
#                            unn.append(j)
#                    else:
#                        count = 0
#                for k in range(len(unn)-1, 0, -1):
#                    del newline[unn[k]]
                #print(newline) 
                newline = ''.join(newline)
                print(newline)
                #newline = line
                
        outfile.write(newline)
if __name__ == "__main__":
    main()