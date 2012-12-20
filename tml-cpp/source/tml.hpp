/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This parser loads an entire TML file into memory very efficiently in both time and space.
 *
 * This C++ "implementation" of a TML parser is actually just a wrapper around the C implementation.
 */

#pragma once
#ifndef _TML_HPP__
#define _TML_HPP__

extern "C" {
	#include "../../tml-c/source/tml_parser.h"
}

#include <string>

#define MAX_TML_STRING_SIZE 4096

// A TmlNode is a "handle" to a node within TML data tree (stored in a TmlData object). You can
// use methods here to traverse the tree, like getFirstChild() and getNextSibling(). When you find
// a node of interest, useful methods like toInt(), toDouble(), toString(), toMarkupString() allow
// you to convert its contents to something you can use.
class TmlNode
{
public:
	TmlNode(struct tml_node n) : node(n) {}
	TmlNode(const TmlNode &c) : node(c.node) {}

	bool isNull()
	{
		return tml_is_node_null(&node);
	}

	bool isLeaf()
	{
		return tml_is_node_leaf(&node);
	}

	std::string getValue()
	{
		return std::string(node.value);
	}

	TmlNode getFirstChild()
	{
		return TmlNode( tml_first_child(&node) );
	}

	TmlNode getNextSibling()
	{
		return TmlNode( tml_next_sibling(&node) );
	}

	int getChildCount()
	{
		return tml_child_count(&node);
	}

	TmlNode getChildAtIndex(int childIndex)
	{
		return TmlNode( tml_child_at_index(&node, childIndex) );
	}

	std::string toString()
	{
		char buff[MAX_TML_STRING_SIZE];
		tml_node_to_string(&node, buff, MAX_TML_STRING_SIZE);
		return std::string(buff);
	}

	std::string toMarkupString()
	{
		char buff[MAX_TML_STRING_SIZE];
		tml_node_to_markup_string(&node, buff, MAX_TML_STRING_SIZE);
		return std::string(buff);
	}

	int toInt()
	{
		return tml_node_to_int(&node);
	}

	float toFloat()
	{
		return tml_node_to_float(&node);
	}

	double toDouble()
	{
		return tml_node_to_double(&node);
	}

private:
	struct tml_node node;
};


// TmlData represents a loaded TML load hierarchy, which was parsed from some file/string/memory.
// Use this class to manage the lifetime of TML data, and as an entry point into the node tree via getRoot().
// Warning: When this object is destroyed, all TmlNode objects derived from it will no longer be valid
// or safe to use (though they will still be safe to destroy).
class TmlData
{
public:
	TmlData(struct tml_data *d) : data(d) {}

	~TmlData()
	{
		tml_free_data(data);
	}

	static TmlData *parseFile(const std::string &filename)
	{
		return new TmlData( tml_parse_file(filename.c_str()) );
	}

	static TmlData *parseString(const std::string &data)
	{
		return new TmlData( tml_parse_string(data.c_str()) );
	}

	static TmlData *parseMemory(const char *buff, size_t buff_size)
	{
		return new TmlData( tml_parse_memory(buff, buff_size) );
	}

	TmlNode getRoot()
	{
		return TmlNode(tml_data_root(data));
	}

	std::string getParseError()
	{
		const char *str = tml_parse_error(data);
		if (!str) return std::string();
		else return std::string(str);
	}

private:
	// copying not allowed
	explicit TmlData(const TmlData &c) { data = NULL; }
	explicit TmlData() { data = NULL; }

	struct tml_data *data;
};






#endif
