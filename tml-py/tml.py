# Copyright (C) 2013 John Judnich
# Released as open-source under The MIT Licence.
#
# This module provides a parser which loads an entire TML file into memory as nested lists of strings.
# This also provides functions to convert nested lists-of-strings objects back into TML code, pattern
# matching functions, and searching functions (to easily locate subtrees that match a given pattern).

import re

splitter = re.compile(r"(\s|\[|\]|\|)")
_inter_open, _inter_close, _inter_divider = "\\o", "\\c", "\\d"


def _replaceMultiple(text, replaceTuples):
    for old, new in replaceTuples:
        text = text.replace(old, new)
    return text


def _convertIntermediateEscapeCodes(text):
    replacements = [("\\[", _inter_open), ("\\]", _inter_close), ("\\|", _inter_divider)]
    return _replaceMultiple(text, replacements)


def _convertFinalEscapeCodes(text):
    replacements = [
        (_inter_open, "["), (_inter_close, "]"), (_inter_divider, "|"),
        ("\\s", " "), ("\\t", "\t"), ("\\n", "\n"), ("\\r", "\r"), ("\\\\", "\\")
    ]
    return _replaceMultiple(text, replacements)


def _tokenize(text):
    text = _convertIntermediateEscapeCodes(text)
    return [x for x in splitter.split(text) if x.strip() != ""]


def _parseList(tokIter, endAtBar):
    try:
        tree, tok = [], tokIter.next()

        while tok != "]":
            if tok == "[":
                subTree, tok = _parseList(tokIter, False)
                tree.append(subTree)

            elif tok == "|":
                if endAtBar: break
                tree = [tree]
                while tok != "]":
                    subTree, tok = _parseList(tokIter, True)
                    tree.append(subTree)
                break

            else:
                tree.append(_convertFinalEscapeCodes(tok))

            tok = tokIter.next()

    except StopIteration:
        return tree, tok
    else:
        return tree, tok


def _parseRoot(tokIter):
    try:
        tok = tokIter.next()
        if tok != "[": raise RuntimeError("TML: Expected open bracket")
    except StopIteration:
        raise RuntimeError("TML: Expected open bracket (empty TML text)")

    tree, _ = _parseList(tokIter, False)

    try: tok = tokIter.next()
    except StopIteration: pass
    else: raise RuntimeError("TML: Expected end of file after root node")
    
    return tree


def _treeToStringGen(tmlTree, brackets):
    if type(tmlTree) is not list: return tmlTree
    siblings = [_treeToStringGen(x, brackets) for x in tmlTree]
    text = reduce((lambda a,b: a+" "+b), siblings)
    if brackets: return "[" + text + "]"
    else: return text


def parse(tmlString):
    """Parses the given TML string and returns the result as a "tree" (nested lists and strings)."""
    tokens = _tokenize(tmlString)
    return _parseRoot(tokens.__iter__())


def toString(tmlTree):
    """Converts a given nested lists/strings "tree" object into valid TML code."""
    return _treeToStringGen(tmlTree, False)


def toMarkupString(tmlTree):
    """Converts a given nested lists/strings "tree" object into a string with each item separated by a space."""
    return _treeToStringGen(tmlTree, True)


def compare(candidate, pattern):
    """Compares a candidate TML tree against a given pattern.
    
    If the pattern contains no wildcards, this
    simply compares the two trees and returns true if identical. Alternately, the pattern may contain
    the "\?" wildcard to match any node (any list or string), or the "\*" wildcard to match any zero
    or more nodes up to the end of the list. NOTE: Both "candidate" and "pattern" are expected to be
    parsed data trees, i.e. results from TML.parse (or your own nested lists/strings if you want)."""
    
    if pattern == "\\?": return True
    if type(candidate) is str: return (candidate == pattern)
    if type(pattern) is str: return False

    for i, c in enumerate(candidate):
        if i >= len(pattern): return False
        if c == "\\*": return True
        if not compare(c, pattern[i]): return False

    if len(candidate) < len(pattern):
        return (pattern[len(candidate)] == "\\*")

    return True


def findFirst(node, pattern):
    """Returns the first child under node which matches the given pattern (by tml.compare).
    
    Returns null if no match is found. NOTE: Both "node" and "pattern" are expected to be
    parsed data trees, i.e. results from tml.parse (or your own nested lists/strings if you want)."""

    for n in node:
        if compare(n, pattern): return n
    return null


def findAll(node, pattern):
    """Returns a list of all children under node which match the given pattern (by tml.compare).
    
    Returns [] if no match is found. NOTE: Both "node" and "pattern" are expected to be
    parsed data trees, i.e. results from tml.parse (or your own nested lists/strings if you want)."""
    
    return [n for n in node if compare(n, pattern)]


    

