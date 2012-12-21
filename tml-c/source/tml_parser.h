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
	char *value; /* will never be NULL, but may be "" */
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

/* Call this to destroy a tml_data object (do NOT just use free() on a tml_data* object) 
 * Warning: Once you call this, all tml_node values derived from this data object will be invalidated. */
void tml_free_data(struct tml_data *data);

/* Returns an error description string if a parse error occurred, or NULL if no errors */
const char *tml_parse_error(const struct tml_data *data);

/* Returns the root node for data represented by the tml_data object */
struct tml_node tml_data_root(const struct tml_data *data);

/* Same as tml_data_root(), but returns as a pointer to tml_node for when this is more convenient. */
const struct tml_node *tml_data_root_ptr(const struct tml_data *data);


/* --------------- NODE ITERATION FUNCTIONS -------------------- */

/* Returns a new tml_node representing the next sibling after this node, if it exists.
 * If no such sibling exists, a null tml_node value will be returned. Test for this
 * null output condition with tml_is_node_null() */
struct tml_node tml_next_sibling(const struct tml_node *node); /* O(1) time */

/* Returns a new tml_node representing the first child under this node, if it exists. 
 * If no such child exists, a null tml_node value will be returned. Test for this
 * null output condition with tml_is_node_null() */
struct tml_node tml_first_child(const struct tml_node *node); /* O(1) time */

/* Returns true if this is a null node. A null node is a special node that doesn't
 * actually exist anywhere within your TML file. It's used to indicate the end of 
 * an iteration. Specifically, when tml_next_sibling() or tml_first_child() are called
 * but no appropriate successor exists, a "null" tml_node value is returned.
 * Use this function to determined whether that is a null node or not. */
bool tml_is_null(const struct tml_node *node); /* O(1) time */

/* Returns true if this node contains one or more children.
 * Equivalent to !tml_is_null(tml_first_child(node)), but slightly faster.
 * NOTE: An empty list "[]" is an exceptional case you should watch out for,
 * where it's technically a list but has no children. So a node having no children
 * doesn't necessarily mean it's not a list. If you want to determine if something 
 * is a list, use tml_is_list() instead. */
bool tml_has_children(const struct tml_node *node); /* O(1) time */

/* Returns true if this node is a list of zero or more subnodes.
 * Equivalent to (strcmp(node->value,"")==0), because lists have no string value.
 * NOTE: A list node doesn't guarantee it will have a child. It's possible that a node
 * is a list node without children (TML allows empty list expressions "[]"). */
bool tml_is_list(const struct tml_node *node); /* O(1) time */

/* Returns the number of children this node contains
 * WARNING: This runs in O(n) time where n is the number of child nodes. */
int tml_child_count(const struct tml_node *node);

/* Returns the nth child of this node indexed by child_index (base 0).
 * WARNING: This runs in O(child_index) linear time.*/
struct tml_node tml_child_at_index(const struct tml_node *node, int child_index);


/* --------------- UTILITY FUNCTIONS (CONVERSION) -------------------- */

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "a b c d".
 * Returns the length of the resulting string. */
size_t tml_node_to_string(const struct tml_node *node, char *dest_str, size_t dest_str_size);

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "[a [b [c]] d]".
 * Returns the length of the resulting string. */
size_t tml_node_to_markup_string(const struct tml_node *node, char *dest_str, size_t dest_str_size);

/* Converts the value of this node into an double value. */
float tml_node_to_float(const struct tml_node *node);

/* Converts the value of this node into an double value */
double tml_node_to_double(const struct tml_node *node);

/* Converts the value of this node into an integer value */
int tml_node_to_int(const struct tml_node *node);

/* Reads a list of float values (e.g. "[0.2 1.5 0.8]") into the given float array,
 * up to a maximum of array_size items. Returns the number of values read. */
int tml_node_to_float_array(const struct tml_node *node, float *array, int array_size);

/* Reads a list of double values (e.g. "[0.2 1.5 0.8]") into the given double array,
 * up to a maximum of array_size items. Returns the number of values read. */
int tml_node_to_double_array(const struct tml_node *node, double *array, int array_size);

/* Reads a list of int values (e.g. "[1 2 3]") into the given int array,
 * up to a maximum of array_size items. Returns the number of values read. */
int tml_node_to_int_array(const struct tml_node *node, int *array, int array_size);


/* --------------- UTILITY FUNCTIONS (COMPARISON / PATTERN MATCHING AND SEARCH) -------------------- */

/* tml_compare_nodes: This is a powerful function which allows you not only to compare two nodes for 
 * value equality, but also match a node against a pattern with wildcards. 
 *
 * For standard equality comparison, this works very straightforwardly. It will return true if both 
 * sides are equivalent (e.g. would return true for left = [1 2 3] and the right = [1 2 3], even if
 * they're from different nodes from the tree or entirely different tml_data objects).
 *
 * Pattern matching allows you to match the left side against a pattern on the right side which may
 * include wildcards. There are two types of wildcards, written as "\*" and "\?" in TML.
 * 
 * The \? wildcard represents any single node - either a list item or a leaf item.
 * The \* wildcard represents zero or more nodes, i.e. it matches anything.
 * 
 * Simple examples:
 *
 * [a b c] matches [\? \? \?]. It also matches [\*], [\? \*], [\? \? \*], and [\? \? \? \*].
 * [a b c] matches [\? b c]. [a b c] matches [a \? \?]. [a b c] does not match [\? hello hi].
 * [a b c] does not match [\? \? \? \?] for example, because the pattern expects four nodes.
 * [a b c] also does not match [\? \?] because the pattern expects exactly two nodes.
 * The pattern [\? \*] expects one or more nodes, which accepts [a], [a b], [a b c], etc.
 * Pattern [\*] expects zero or more nodes. So this would match [], [a], [a b], [a b c], etc.
 *
 * Limitation: For performance and simplicity, you cannot write patterns with anything in
 * a list following the "\*" wildcard. For example [a b \*] is valid, but [\* a b] is not.
 * Similarly, [\? \*] is valid, but [\* \?] is not.
 *
 * Nested list patterns:
 * 
 * Keep in mind that each matched node can be lists too, so for example:
 * [\?] matches [[a b c]], because the \? wildcard matches the single nested list.
 *
 * Your patterns can be nested as deeply as you like, and can contain non-wildcard items of 
 * course to match against. For example, say we want to match TML code like
 * "[[bold] [hello this is a test]]" (or equivalently "[bold | hello this is a test]")
 * in such a way that the right nested list can contain anything at all. So the same
 * pattern should also match "[bold | pattern matching is fun]". The following pattern
 * achieves this: "[bold | \*]". Pretty simple and easy once you understand how it works.
 * In this case "[bold | \*]" (which could also be written "[[bold] [\*]]" with absolutely
 * no functional difference) matches any nodes that contain a nested "[bold]" on the left
 * and a nested list of anything on the right.
 *
 * More examples:
 *
 * [bold | hi how are you?] matches [bold | \*]
 * [position | \? \? \?] matches [position | 2.9 3.7 1.0] and [position | 1 2 3]
 * [ vertices | [1 2 3] [5 6 7] [7 8 9] ] matches [vertices | \*]
 *
 * (etc.)
 */
bool tml_compare_nodes(const struct tml_node *candidate, const struct tml_node *pattern);

/* Finds the first child under this node that matches the given pattern, if any.
 * For example searching [1 2 [a 3] [a 4] 5] for the pattern [a \?] will return a node
 * pointing to the [a 3] node, because it's the first child that matches [c \?]. Then,
 * if you want to iterate to the next one "[a 4]", use tml_find_next_sibling.
 * See tml_compare_nodes() for more info on how pattern matching works. 
 * NOTE that this will NOT return node itself even if it matches the pattern. For example
 * if you tml_find_first_child() with node [a 4] and pattern [a \?], you'll get a NULL
 * node as a result because [a 4] does not contain any child node of the specified pattern.
 * If you want to check if a node itself matches a pattern (not its children), use
 * tml_compare_nodes(). */
struct tml_node tml_find_first_child(const struct tml_node *node, const struct tml_node *pattern);

/* Finds the next sibling after this node that matches the given pattern, if any.
 * See tml_compare_nodes() for more info on how pattern matching works. */
struct tml_node tml_find_next_sibling(const struct tml_node *node, const struct tml_node *pattern);



#endif
