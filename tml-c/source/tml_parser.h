
#pragma once
#ifndef _TML_PARSER_H__
#define _TML_PARSER_H__

#include <ctype.h>
#include <stdbool.h>

struct tml_node
{
	size_t next_sibling;
	size_t first_child;
	/* NULL-terminated C string value of a leaf node (NULL if not a leaf) */
	char *value;
};

struct tml_data
{
	struct tml_node root_node;
	char *buff;
	size_t buff_size, buff_allocated;
	const char *error_msg;
};


/* Create a new tml_data object, parsing from TML text contained within the given memory buffer. */
struct tml_data *tml_parse_memory(char *buff, size_t buff_size);

/* Create a new tml_data object, parsing from TML text contained within the given C string. */
struct tml_data *tml_parse_string(char *str);

/* Create a new tml_data object, parsing from TML text from the file specified (by filename). */
struct tml_data *tml_parse_file(char *filename);

/* Call this to destroy a tml_data object. */
void tml_free_data(struct tml_data *data);


/* Returns an error description string if a parse error occurred, or NULL if no errors */
const char *tml_parse_error(struct tml_data *data);

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

/* Returns true if this is a leaf node. Leav nodes have no children and have a string value.
 * This string value can be retrieved using tml_node_to_string() to copy it to some destination,
 * or directly as a C string from (char*)node->value. */
bool tml_is_node_leaf(struct tml_node *node); /* O(1) time */

/* Converts the contents of this node into an double value */
double tml_node_to_double(struct tml_node *node);

/* Converts the contents of this node into an float value */
float tml_node_to_float(struct tml_node *node);

/* Converts the contents of this node into an integer value */
int tml_node_to_int(struct tml_node *node);

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "a b c d". */
void tml_node_to_string(struct tml_node *node, char *dest_str, size_t dest_str_size);

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "[a [b [c]] d]". */
void tml_node_to_markup_string(struct tml_node *node, char *dest_str, size_t dest_str_size);



#endif
