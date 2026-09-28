"""List code symbols (publics and virtual segments) in a compiled source file.

Usage: syms.py FILE.cpp     compile it, then print size and mangled name for use in // MATCH
"""
import os, sys
sys.path.insert(0, os.path.dirname(__file__))
import match

for src in sys.argv[1:]:
    obj = match.compile_(os.path.abspath(src), match.DEFAULT_FLAGS)
    for name, data, mask in match.code_symbols(obj):
        print('%6d  %s' % (len(data), name))
