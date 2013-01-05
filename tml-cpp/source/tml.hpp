/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This parser loads an entire TML file into memory very efficiently in both time and space.
 *
 * This C++ "implementation" of a TML parser is actually just a wrapper around the C implementation.
 * Refer to the C implementation header ("tml-c/source/tml_parser.h") for detailed API documentation.
 */

#pragma once
#ifndef _TML_HPP__
#define _TML_HPP__

extern "C" {
	#include "../../tml-c/source/tml_parser.h"
}

#include <string>

#define MAX_TML_STRING_SIZE 4096

class TmlDoc;
class TmlNode;

// A TmlNode is a "handle" to a node within TML tml_doc tree (stored in a TmlData object). You can
// use methods here to traverse the tree, like getFirstChild() and getNextSibling(). When you find
// a node of interest, useful methods like toInt(), toDouble(), toString(), toMarkupString() allow
// you to convert its contents to something you can use.
class TmlNode
{
public:
	TmlNode() : node(TML_NODE_NULL) {}
	TmlNode(const struct tml_node n) : node(n) {}
	TmlNode(const TmlNode &c) : node(c.node) {}

	bool isNull() const
	{
		return tml_is_null(&node);
	}

	bool hasChildren() const
	{
		return tml_has_children(&node);
	}

	bool isList() const
	{
		return tml_is_list(&node);
	}

	std::string getValue() const
	{
		return std::string(node.value);
	}

	// Slightly faster than getValue() due to no extra copy operation.
	// Useful for iteration/comparisons, since you can compare C strings to C++ strings.
	const char *getValueCstr() const
	{
		return node.value;
	}

	TmlNode getFirstChild() const
	{
		return TmlNode( tml_first_child(&node) );
	}

	TmlNode getNextSibling() const
	{
		return TmlNode( tml_next_sibling(&node) );
	}

	// WARNING: This runs in O(n) time where n is the number of child nodes.
	int getChildCount() const
	{
		return tml_child_count(&node);
	}

	// WARNING: This runs in O(n) time where n is the number of child nodes.
	TmlNode getChildAtIndex(int childIndex) const
	{
		return TmlNode( tml_child_at_index(&node, childIndex) );
	}

	// Same as getChildAtIndex().
	// WARNING: This runs in O(n) time where n is the number of child nodes.
	TmlNode operator[] (int childIndex) const
	{
		return TmlNode( tml_child_at_index(&node, childIndex) );
	}

	std::string toString() const
	{
		char buff[MAX_TML_STRING_SIZE];
		size_t len = tml_node_to_string(&node, buff, MAX_TML_STRING_SIZE);
		return std::string(buff, len);
	}

	std::string toMarkupString() const
	{
		char buff[MAX_TML_STRING_SIZE];
		tml_node_to_markup_string(&node, buff, MAX_TML_STRING_SIZE);
		return std::string(buff);
	}

	int toInt() const
	{
		return tml_node_to_int(&node);
	}

	float toFloat() const
	{
		return tml_node_to_float(&node);
	}

	double toDouble() const
	{
		return tml_node_to_double(&node);
	}

	int toIntArray(int *array, int arraySize)
	{
		return tml_node_to_int_array(&node, array, arraySize);
	}

	int toFloatArray(float *array, int arraySize)
	{
		return tml_node_to_float_array(&node, array, arraySize);
	}

	int toDoubleArray(double *array, int arraySize)
	{
		return tml_node_to_double_array(&node, array, arraySize);
	}

	bool compareToPattern(const TmlNode &pattern) const
	{
		return tml_compare_nodes(&node, &pattern.node);
	}

	TmlNode findFirstChild(const TmlNode &pattern) const
	{
		return TmlNode( tml_find_first_child(&node, &pattern.node) );
	}

	TmlNode findNextSibling(const TmlNode &pattern) const
	{
		return TmlNode( tml_find_next_sibling(&node, &pattern.node) );
	}

	bool compareToPattern(const TmlDoc *patternData) const;
	TmlNode findFirstChild(const TmlDoc *patternData) const;
	TmlNode findNextSibling(const TmlDoc *patternData) const;

	// Be careful, the following overloads are potentially inefficient because they allocate, parse, and
	// free the pattern string these functions are used.

	bool compareToPattern(const std::string &patternStr) const;
	TmlNode findFirstChild(const std::string &patternStr) const;
	TmlNode findNextSibling(const std::string &patternStr) const;

private:
	struct tml_node node;
};


// TmlDoc represents a loaded TML load hierarchy, which was parsed from some file/string/memory.
// Use this class to manage the lifetime of TML data, and as an entry point into the node tree via getRoot().
// Warning: When this object is destroyed, all TmlNode objects derived from it will no longer be valid
// or safe to use (though they will still be safe to destroy).
class TmlDoc
{
public:
	TmlDoc(struct tml_doc *d) : data(d)
	{
		if (data == NULL)
			throw "Error instantiating tml_doc";
	}

	TmlDoc(std::string dataStr)
	{
		data = tml_parse_string(dataStr.c_str());
		if (data == NULL)
			throw "Error instantiating tml_doc";
	}

	~TmlDoc()
	{
		tml_free_doc(data);
	}

	static TmlDoc *parseFile(const std::string &filename)
	{
		return new TmlDoc( tml_parse_file(filename.c_str()) );
	}

	static TmlDoc *parseString(const std::string &data)
	{
		return new TmlDoc( tml_parse_string(data.c_str()) );
	}

	static TmlDoc *parseMemory(const char *buff, size_t buff_size)
	{
		return new TmlDoc( tml_parse_memory(buff, buff_size) );
	}

	TmlNode getRoot() const
	{
		return TmlNode(data->root_node);
	}

	std::string getParseError() const
	{
		const char *str = data->error_message;
		if (!str) return std::string();
		else return std::string(str);
	}

private:
	explicit TmlDoc(const TmlDoc &c) { throw "Copying TmlDoc not allowed"; }
	explicit TmlDoc() { throw "Empty TmlDoc not allowed"; }

	struct tml_doc *data;
};



bool TmlNode::compareToPattern(const TmlDoc *patternData) const
{
	TmlNode pattern = patternData->getRoot();
	return compareToPattern(pattern);
}

TmlNode TmlNode::findFirstChild(const TmlDoc *patternData) const
{
	TmlNode pattern = patternData->getRoot();
	return findFirstChild(pattern);
}

TmlNode TmlNode::findNextSibling(const TmlDoc *patternData) const
{
	TmlNode pattern = patternData->getRoot();
	return findNextSibling(pattern);
}


bool TmlNode::compareToPattern(const std::string &patternStr) const
{
	TmlDoc patternData(patternStr);
	TmlNode pattern = patternData.getRoot();
	return compareToPattern(pattern);
}

TmlNode TmlNode::findFirstChild(const std::string &patternStr) const
{
	TmlDoc patternData(patternStr);
	TmlNode pattern = patternData.getRoot();
	return findFirstChild(pattern);
}

TmlNode TmlNode::findNextSibling(const std::string &patternStr) const
{
	TmlDoc patternData(patternStr);
	TmlNode pattern = patternData.getRoot();
	return findNextSibling(pattern);
}



#endif
