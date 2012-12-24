/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This module provides a parser which loads an entire TML file into memory as nested lists of strings.
 * This also provides functions to convert nested lists-of-strings objects back into TML code, pattern
 * matching functions, and searching functions (to easily locate subtrees that match a given pattern).
 */

var TML = (function () {
    "use strict";
    var me = {};

    var collapsed_OpenBracket = "\\o",
        collapsed_CloseBracket = "\\c",
        collapsed_Divider = "\\d";

    var convertIntermediateEscapeCodes = function (str) {
        var result = str.replace(/\\./g, function (code) {
            if (code === "\\[") return collapsed_OpenBracket;
            if (code === "\\]") return collapsed_CloseBracket;
            if (code === "\\|") return collapsed_Divider;
            return code;
        });
        return result;
    };

    var convertFinalEscapeCodes = function (str) {
        var result = str.replace(/\\./g, function (code) {
            if (code === collapsed_Divider) return "|";
            if (code === collapsed_OpenBracket) return "[";
            if (code === collapsed_Divider) return "]";
            if (code === "\\s") return " ";
            if (code === "\\t") return "\t";
            if (code === "\\n") return "\n";
            if (code === "\\r") return "\r";
            if (code === "\\\\") return "\\";
            return code;
        });
        return result;
    };

    // Parses list "... ]" where the open bracket has already been read.
    // If endAtBar is true, this will let "|" terminate the list.
    var parseList = function (tokens, index, endAtBar) {
        var tokLen = tokens.length;
        var list = [];
        var i = index;

        while (i < tokLen) {
            var tok = tokens[i++];

            if (tok === "[") {
                // append sublist
                var ret = parseList(tokens, i);
                i = ret.index;
                list.push(ret.result);
            } 
            else if (tok === "|") {
                if (endAtBar)
                    return {index: i, result: list};

                // nest-ify
                list = [list];
                while (i < tokLen) {
                    var ret = parseList(tokens, i, true);
                    i = ret.index;

                    list.push(ret.result);

                    if (tokens[i - 1] === "]")
                        return {index: i, result: list};
                }
                break; // expected closing bracket, reached EOF
            }
            else if (tok === "]") {
                // close this list
                return {index: i, result: list};
            }
            else {
                // append word
                if (tok !== "")
                    list.push(convertFinalEscapeCodes(tok));
            }
        }

        throw "TML: Expected close bracket";
    };

    var parseRoot = function (tokens) {
        var tokLen = tokens.length;
        if (tokLen === 0 || tokens[0] !== "[")
            throw "TML: Expected open bracket";

        var ret = parseList(tokens, 1, false);
        var tree = ret.result;

        if (ret.index < tokLen)
            throw "TML: Expected end of file after root node";

        return tree;
    };


    var treeToStringGen = function (tmlTree, brackets) {
        if (typeof tmlTree === "string") return tmlTree;

        var len = tmlTree.length, str = "";
        if (brackets) str = "[";

        for (var i = 0; i < len; ++i) {
            var v = tmlTree[i];

            if (typeof(v) === "string")
                str += v;
            else
                str += treeToStringGen(v, brackets);

            if (i < len-1) str += " ";
        }

        if (brackets) str += "]";
        return str;
    };


    // Parses the given TML string and returns the result a a "tree" (nested Javascript lists and strings).
    me.parse = function (tmlString) {
        var rawString = convertIntermediateEscapeCodes(tmlString);
        var tokens = rawString.match(/((\w|\\.)+|\[|\]|\|)/g);
        return parseRoot(tokens);
    };

    // Converts a given Javascript nested lists/strings "tree" object into valid TML code
    me.toMarkupString = function (tmlTree) {
        return treeToStringGen(tmlTree, true);
    };

    // Converts a given Javascript nested lists/strings "tree" object into a string with each item separated by a space
    me.toString = function (tmlTree) {
        return treeToStringGen(tmlTree, false);
    };

    // Compares a candidate TML tree against a given pattern. If the pattern contains no wildcards, this
    // simply compares the two trees and returns true if identical. Alternately, the pattern may contain 
    // the "\?" wildcard to match any node (any list or string), or the "\*" wildcard to match any zero
    // or more nodes up to the end of the list.
    me.compare = function (candidate, pattern) {
        if (pattern === "\\?")
            return true;
        if (typeof candidate === "string")
            return (candidate === pattern);
        if (typeof pattern === "string") 
            return false;

        var cLen = candidate.length, pLen = pattern.length, c;
        for (c = 0; c < cLen; ++c) {
            if (c >= pLen) 
                return false;
            if (pattern[c] === "\\*") 
                return true;
            if (!me.compare(candidate[c], pattern[c])) 
                return false;
        }

        if (c < pLen) {
            if (pattern[c] === "\\*")
                return true;
            else
                return false;
        }

        return true;
    };

    // Returns the first child under node which matches the given pattern (as determined by TML.compare).
    // Returns null if no match is found.
    me.find = function (node, pattern) {
        var len = node.length;
        for (var i = 0; i < len; ++i) {
            if (me.compare(node[i], pattern))
                return node[i];
        }
        return null;
    };

    // Returns a list of all children under node which match the given pattern (as determined by TML.compare).
    // Returns [] if no match is found.
    me.findAll = function (node, pattern) {
        var results = [];
        var len = node.length;
        for (var i = 0; i < len; ++i) {
            if (me.compare(node[i], pattern))
                results.push(node[i]);
        }
        return results;
    };

    return me;
}());



