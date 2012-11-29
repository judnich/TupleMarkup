#pragma once

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
};

struct tml_data *tml_parse_memory(char *data, size_t data_size);
struct tml_data *tml_parse_string(char *str);
struct tml_data *tml_parse_file(char *filename);

void tml_free_data(struct tml_data *data);

struct tml_node tml_next_sibling(struct tml_node *node); /* O(1) time */
struct tml_node tml_first_child(struct tml_node *node); /* O(1) time */

int tml_child_count(struct tml_node *node); /* O(n) time */
struct tml_node tml_child_at_index(struct tml_node *node, int child_index); /* O(n) time */

bool tml_is_node_null(struct tml_node *node); /* O(1) time */
bool tml_is_node_leaf(struct tml_node *node); /* O(1) time */

double tml_node_to_double(struct tml_node *node);
float tml_node_to_float(struct tml_node *node);
int tml_node_to_int(struct tml_node *node);

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "a b c d". */
void tml_node_to_string(struct tml_node *node, char *dest_str, size_t dest_str_size);

/* Converts the contents of this node into a string, minus TML notation. For example if
 * the node represents the subtree "[a [b [c]] d]", the result will be "[a [b [c]] d]". */
void tml_node_to_markup_string(struct tml_node *node, char *dest_str, size_t dest_str_size);

