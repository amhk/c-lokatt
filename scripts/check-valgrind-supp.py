#!/usr/bin/python3
import argparse


def warn(path: str, lineno_dup: int, lineno_orig: int, block: str):
    print('{:s}:{:d}: duplicate block originally defined at {:s}:{:d}:\n{:s}'
          .format(path, lineno_dup, path, lineno_orig, block.rstrip()))


def check_path(path: str):
    blocks = dict()
    with open(path) as f:
        current = None
        for lineno, line in enumerate(f, start=1):
            if line == '}\n':
                current[0] += line

                if current[0] not in blocks:
                    blocks[current[0]] = current[1]
                else:
                    warn(path, current[1], blocks[current[0]], current[0])
                current = None
                continue
            if line == '{\n':
                current = [line, lineno]
                continue
            if current is not None:
                if current[0] == '{\n':
                    line = '   <title>\n'  # don't factor in the block title
                current[0] += line
                continue


def parse_args():
    parser = argparse.ArgumentParser(
        description='Check Valgrind suppressions file(s) for errors',
        epilog='Note: currently only checks for duplicate suppressions '
               'in the same file.')
    parser.add_argument('path', nargs='+')
    return parser.parse_args()


def main():
    args = parse_args()
    for path in args.path:
        check_path(path)


if __name__ == '__main__':
    main()
