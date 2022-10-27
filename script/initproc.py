import os
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('INIT_PROC', default="usershell")
    parser.add_argument('ASM', default="script")
    args = parser.parse_args()
    f = open("../" + args.ASM + "/initproc.S", mode="w")
    f.write(
'''
    .global INIT_PROC
INIT_PROC:
    .string \"{0}\"
'''.format(args.INIT_PROC));
