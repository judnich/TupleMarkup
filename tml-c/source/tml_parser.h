/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This parser loads an entire TML file into memory very efficiently in both time and space.
 * 
 * Storage space overhead is very low, with only 1 extra byte per leaf node and exactly 10 bytes
 * for nonleaf (list) nodes. Malloc is called only once, and realloc is rarely used.
 *
 * The parsing process consists of reading from the token stream and writing variable length
 * node data (along with leaf node contents strings) into one big array buffer. All node data is
 * contained within this buffer, so malloc isn't used except to initially create this buffer.
 */

#pragma once
#ifndef _TML_PARSER_H__
#define _TML_PARSER_H__

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

/* We use this typedef because 32 bit offset values are fine for any TML file under 4 GB.
 * If you need to load TML files over 4 GB into memory, then this can be changed larger. */
typedef uint32_t tml_offset_t; 
/* This should be kept consistent to be 2 to the power of sizeof(tml_offset_t) */
#define TML_PARSER_MAX_DATA_SIZE 0xFFFF

struct tml_node
{
	char *buff;
	tml_offset_t next_sibling;
	tml_offset_t first_child;
	char *value;
};

struct tml_data
{
	struct tml_node root_node;
	char *buff;
	size_t buff_index, buff_allocated;
	const char *error_msg;
};

/* --------------- DATA PARSE FUNCTIONS -------------------- */

/* Create a new tml_data object, parsing from TML text contained within the given C string. */
struct tml_data *tml_parse_string(const char *str);

/* Create a new tml_data object, parsing from TML text from the file specified (by filename). */
struct tml_data *tml_parse_file(const char *filename);

/* Create a new tml_data object, parsing from TML text contained within the given memory buffer.
 * The parsing procedure allocates its own memory for parsed data, so you can delete your "buff"
 * data right after calling this if you want. */
struct tml_data *tml_parse_memory(const char *buff, size_t buff_size);

/* Create a new tml_data object, parsing from TML text contained within the given memory buffer,
 * using the given memory buffer as a parser working space to conserve memory (less malloc's). This 
 * means that your "buff" data may be modified by the parsing process, so consider the data invalidated 
 * after calling this. The parsing procedure creates its own internal memory for parsed data, so you can
 * delete the "buff" data right after calling this. */
struct tml_data *tml_parse_in_memory(char *buff, size_t buff_size);

/* Call this to destroy a tml_data object (do NOT just use free() on a tml_data* object) */
void tml_free_data(struct tml_data *data);

/* Returns an error description string if a parse error occurred, or NULL if no errors */
const char *tml_parse_error(struct tml_data *data);

/* Returns the root node for data represented by the tml_data object */
struct tml_node tml_data_root(struct tml_data *data);


/* --------------- NODE ITERATION FUNCTIONS -------------------- */

/* Returns a new tml_node representing the next sibling after this node, if it exists.
 * If no such sibling exists, a null tml_node value will be returned. Test for this
 * null output condition with tml_is_node_null() */
struct tml_node tml_next_sibling(struct tml_node *node); /* O(1) time */

/* Returns a new tml_node representing the first child under this node, if it exists. 
 * If no such child exists, a null tml_node value will be returned. Test for this
 * null output condition with tml_is_node_null() */
struct tml_node tml_first_child(struct tml_node *node); /* O(1) time */

/* Returns the number of children this node contains */
int tml_child_count(struct tml_node *node); /* O(n) time */

/* Returns the nth child of this node indexed by child_index (base 0) */
struct tml_node tml_child_at_index(struct tml_node *node, int child_index); /* O(n) time */

/* Returns true if this is a null node. When tml_next_sibling() or tml_first_child()  
 * are called but no appropriate successor exists, a "null" tml_node value is returned.
 * Use this function to determined whether that is a null node or not. */
bool tml_is_node_null(struct tml_node *node); /* O(1) time */

/* Returns true if this is a leaf node. Leaf nodes have no children and may have a string value.
 * If your TML file contained a null list "[]", then it will appear as a leaf node with no string value.
 * Otherwise, the string value can be retrieved using tml_node_to_string() to copy it to some destination,
 * or directly as a C string from (char*)node->value. */
bool tml_is_node_leaf(struct tml_node *node); /* O(1) time */


/* --------------- UTILITY FUNCTIONS (CONVERSION) -------------------- */

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "a b c d".
 * Returns the length of the resulting string. */
size_t tml_node_to_string(struct tml_node *node, char *dest_str, size_t dest_str_size);

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "[a [b [c]] d]".
 * Returns the length of the resulting string. */
size_t tml_node_to_markup_string(struct tml_node *node, char *dest_str, size_t dest_str_size);

/* Converts the contents of this node into an double value */
double tml_node_to_double(struct tml_node *node);

/* Converts the contents of this node into an float value */
float tml_node_to_float(struct tml_node *node);

/* Converts the contents of this node into an integer value */
int tml_node_to_int(struct tml_node *node);


/* --------------- UTILITY FUNCTIONS (COMPARISON / PATTERN MATCHING AND SEARCH) -------------------- */

/* This is a powerful function which allows you not only to compare two nodes for value equality, but
 * also match a node against a pattern with various types of wildcards. For standard equality comparison,
 * this works very straightforwardly. It will return true if both sides are equivalent (e.g. if the left
 * represents [1 2 3] and the right also represents [1 2 3], even if they're from different nodes or
 * entirely different tml_data objects). Pattern matching allows you to match the left side against a
 * pattern on the right side which may include wildcards. TODO: More documentation on this. */
/*TODO:bool tml_compare_nodes(struct tml_node *candidate, struct tml_node *pattern);*/

/* Finds the next sibling after this node that matches the given pattern, if any.
 * See tml_compare_nodes() for more info on how pattern matching works. */
/*TODO:struct tml_node tml_find_next_sibling(struct tml_node *node, struct tml_node *pattern);*/

/* Finds the first child under this node that matches the given pattern, if any.
 * See tml_compare_nodes() for more info on how pattern matching works. */
/*TODO:struct tml_node tml_find_first_child(struct tml_node *node, struct tml_node *pattern);*/



#endif
