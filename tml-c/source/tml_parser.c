/* 
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence. 
 *
 * TML Parser - C Implementation
 *
 * Notes: Data is parsed and inserted into memory using a packed format to reduce the 
 * overhead that would otherwise be incurred by the tree linkings (~10 bytes per node).
 * This format compresses that down to only 1 byte overhead per node for leaf nodes.
 *
 * A node begins with the pointer data, followed by a null terminated value string IF
 * the pointer data indicates that this node is a leaf node.
 *
 * The pointer data is variable length, and has two forms:
 *
 * 1) One form is a single byte with value 0-254. In this case this value represents 
 * the relative offset to skip within the parsed data buffer to find the next subling node
 * (or 0 if no next sibling), and also indicates that this is a leaf node.
 *
 * 2) If the first byte is 255 then the next following bytes are a next_sibling
 * absolute offset followed by a first_child absolute offset.
 */

#include "tml_parser.h"
#include "tml_tokenizer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static void set_parse_error(struct tml_data *data, const char *error_msg);
static void parse_root(struct tml_data *data, struct tml_stream *tokens);
static size_t parse_list_node(struct tml_data *data, struct tml_stream *tokens, bool process_divider, struct tml_token *token_out);


#define FULL_NODE_DATA_FLAG 0xFF
static const int NODE_LINK_DATA_SIZE = sizeof(char) + sizeof(tml_offset_t)*2;

static struct tml_node NULL_NODE = { 0, 0, 0, "" };


static void grow_buffer_if_needed(struct tml_data *data, size_t new_size)
{
	if (new_size >= TML_PARSER_MAX_DATA_SIZE) {
		set_parse_error(data, 
			"TML data file is too large, parsed data structures exceeded TML_PARSER_MAX_DATA_SIZE.");
	}

	if (new_size > data->buff_allocated && data->buff) {
		data->buff_allocated *= 2;
		data->buff = realloc(data->buff, data->buff_allocated);
	}
}

static void shrink_buffer(struct tml_data *data)
{
	if (data->buff) {
		data->buff_allocated = data->buff_index;
		data->buff = realloc(data->buff, data->buff_allocated);
	}
}


struct tml_data *tml_parse_in_memory(char *ibuff, size_t ibuff_size)
{
	struct tml_data *data = malloc(sizeof(*data));
	if (!data) return NULL;

	memset(data, 0, sizeof(*data));

	data->error_msg = NULL;
	data->buff_index = 0;
	data->buff_allocated = ibuff_size * 2;
	data->buff = malloc(data->buff_allocated);

	if (!data->buff) {
		free(data);
		return NULL;
	}

	struct tml_stream tokens = tml_stream_open(ibuff, ibuff_size);
	parse_root(data, &tokens);
	tml_stream_close(&tokens);

	shrink_buffer(data);

	if (data->buff == NULL) {
		/* buff is NULL if realloc has failed */
		free(data);
		return NULL;
	}

	data->root_node.buff = data->buff;

	return data;
}

struct tml_data *tml_parse_memory(const char *ibuff, size_t ibuff_size)
{
	char *ibuff_copy = malloc(ibuff_size);
	if (!ibuff_copy) return NULL;
	memcpy(ibuff_copy, ibuff, ibuff_size);
	struct tml_data *data = tml_parse_in_memory(ibuff_copy, ibuff_size);
	free(ibuff_copy);
	return data;
}

struct tml_data *tml_parse_string(const char *str)
{
	size_t len = strlen(str);
	return tml_parse_memory(str, len);
}

struct tml_data *tml_parse_file(const char *filename)
{
	long int fsize;

	FILE *fp = fopen(filename, "rb");
	if (!fp)
		return NULL;

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp); /* get file size */
	rewind(fp);

	char *buff = malloc(sizeof(char) * fsize);
	if (!buff) {
		fclose(fp);
		return NULL;
	}

	if (fsize != fread(buff, 1, fsize, fp)) {
		fclose(fp);
		free(buff);
		return NULL;
	}

	struct tml_data *data = tml_parse_in_memory(buff, fsize);

	fclose(fp);
	free(buff);

	return data;
}

void tml_free_data(struct tml_data *data)
{
	if (data) {
		if (data->buff)
			free(data->buff);
		free(data);
	}
}

static void set_parse_error(struct tml_data *data, const char *error_msg)
{
	if (data->error_msg == NULL)
		data->error_msg = error_msg;
}

const char *tml_parse_error(const struct tml_data *data)
{
	return data->error_msg;
}

struct tml_node tml_data_root(const struct tml_data *data)
{
	return data->root_node;
}

const struct tml_node *tml_data_root_ptr(const struct tml_data *data)
{
	return &data->root_node;
}



static void write_packed_node(struct tml_data *data, const char *str, int str_len, int sibling_offset)
{
	size_t index = data->buff_index;

	grow_buffer_if_needed(data, 
		index + sizeof(char) + (str_len + 1) * sizeof(char));

	if (data->buff == NULL) return; /* in case realloc fails */

	/* write sibling offset byte */
	((unsigned char*)data->buff)[index] = (unsigned char)sibling_offset;
	index += sizeof(unsigned char);

	/* copy string contents */
	if (str_len > 0) {
		memcpy(data->buff + index, str, str_len * sizeof(char));
		index += str_len * sizeof(char);
	}

	/* null terminate string */
	data->buff[index] = '\0';
	index += sizeof(char);

	data->buff_index = index;
}

static size_t write_node(struct tml_data *data, const char *str, int str_len)
{
	size_t index = data->buff_index;

	grow_buffer_if_needed(data, 
		index + NODE_LINK_DATA_SIZE + (str_len + 1) * sizeof(char));

	if (data->buff == NULL) return 0; /* in case realloc fails */

	/* write node link data */
	char *ptr = &data->buff[index];
	ptr[0] = FULL_NODE_DATA_FLAG;
	memset(ptr+1, 0, NODE_LINK_DATA_SIZE-1);
	index += NODE_LINK_DATA_SIZE;

	/* copy string contents */
	if (str_len > 0) {
		memcpy(data->buff + index, str, str_len * sizeof(char));
		index += str_len * sizeof(char);
	}

	/* null terminate string */
	data->buff[index] = '\0';
	index += sizeof(char);

	data->buff_index = index;
	return ptr - data->buff;
}

static __inline__ void update_node_child(char *node_ptr, size_t first_child)
{
	tml_offset_t *fc = (tml_offset_t*)(node_ptr + 1);
	*fc = (tml_offset_t)first_child;
}

static __inline__ void update_node_sibling(char *node_ptr, size_t next_sibling)
{
	tml_offset_t *ns = (tml_offset_t*)(node_ptr + 1 + sizeof(tml_offset_t));
	*ns = (tml_offset_t)next_sibling;
}

static __inline__ size_t get_node_child(char *node_ptr)
{
	tml_offset_t *fc = (tml_offset_t*)(node_ptr + 1);
	return *fc;
}

static __inline__ size_t get_node_sibling(char *node_ptr)
{
	tml_offset_t *ns = (tml_offset_t*)(node_ptr + 1 + sizeof(tml_offset_t));
	return *ns;
}


/* Parses "[...]" */
static void parse_root(struct tml_data *data, struct tml_stream *tokens)
{
	struct tml_token token = tml_stream_pop(tokens);

	if (token.type != TML_TOKEN_OPEN) {
		if (token.type == TML_TOKEN_EOF)
			set_parse_error(data, "File contents is empty");
		else
			set_parse_error(data, "Expecting opening bracket at start of file");
		data->root_node = NULL_NODE;
		return;
	}

	size_t root_node_offset = parse_list_node(data, tokens, true, NULL);

	token = tml_stream_pop(tokens);

	if (token.type != TML_TOKEN_EOF) {
		set_parse_error(data, "Expected end of file after end of root node");
		return;
	}

	if (data->buff) {
		data->root_node.buff = data->buff;
		data->root_node.value = "";
		data->root_node.next_sibling = 0;
		data->root_node.first_child = get_node_child(&data->buff[root_node_offset]);
	}

	return;
}


/* Parses "...]", a list where we assume that the opening brace has been read. 
 * After returning from this function, the stream will have read the closing brace. */
static size_t parse_list_node(struct tml_data *data, struct tml_stream *tokens, bool process_divider, struct tml_token *token_out)
{
	/* this is the container node for the list contents */
	size_t root_node = write_node(data, NULL, 0);

	struct tml_token token, last_token;
	bool peeked = false;
	bool set_first_child = false;

	for (;;) {
		/* read the next token, if we haven't already peeked at it */
		if (!peeked) {
			last_token = token;
			token = tml_stream_pop(tokens);
		} else {
			peeked = false;
		}

		if (token.type == TML_TOKEN_ITEM) {
			/* record the first child under this list */
			if (!set_first_child) {
				update_node_child(&data->buff[root_node], data->buff_index);
				set_first_child = true;
			}

			/* peek ahead to see if there is a next sibling */
			last_token = token;
			token = tml_stream_pop(tokens);
			peeked = true;

			if (token.type != TML_TOKEN_ITEM && token.type != TML_TOKEN_OPEN) {
				/* this is the last element of a list, so use next_sibling offset = 0 */
				write_packed_node(data, last_token.value, last_token.value_size, 0);
			}
			else {
				/* this is a regular leaf node with a next sibling */
				if (last_token.value_size < FULL_NODE_DATA_FLAG) {
					/* length of this leaf node string is under 255 characters */
					int sibling_offset = last_token.value_size;
					write_packed_node(data, last_token.value, last_token.value_size, sibling_offset);
				}
				else {
					/* length of contents exceeds 255 characters so use full 32-bit node link data */
					size_t n = write_node(data, last_token.value, last_token.value_size);
					update_node_sibling(&data->buff[n], data->buff_index);
				}
			}
		}
		else if (token.type == TML_TOKEN_OPEN) {
			/* record the first child under this list */
			if (!set_first_child) {
				update_node_child(&data->buff[root_node], data->buff_index);
				set_first_child = true;
			}

			/* recurse into a new list item */
			size_t list_node = parse_list_node(data, tokens, true, NULL);

			/* peek ahead to see if there is a next sibling */
			last_token = token;
			token = tml_stream_pop(tokens);
			peeked = true;

			/* set the list's next node link the current writing position if there are more list items */
			if (token.type == TML_TOKEN_ITEM || token.type == TML_TOKEN_OPEN) {
				update_node_sibling(&data->buff[list_node], data->buff_index);
			}
		}
		else if (token.type == TML_TOKEN_DIVIDER) {
			if (!process_divider) {
				break;
			}

			/* make the already written items into a list */
			size_t first_list = write_node(data, NULL, 0);
			update_node_child(&data->buff[first_list], get_node_child(&data->buff[root_node]));
			update_node_sibling(&data->buff[first_list], data->buff_index);
			update_node_child(&data->buff[root_node], first_list);

			/* recurse reading the list of nested items */
			for (;;) {
				struct tml_token closing_token;
				size_t list_node = parse_list_node(data, tokens, false, &closing_token);

				if (closing_token.type == TML_TOKEN_DIVIDER)
					update_node_sibling(&data->buff[list_node], data->buff_index);
				else
					break;
			}

			/* we've read the last nested list plus its closing ']', so all done */
			break;
		}
		else if (token.type == TML_TOKEN_CLOSE || token.type == TML_TOKEN_EOF) {
			break;
		}

		/* in case out of memory */
		if (!data->buff) {
			return 0;
		}
	}

	if (token.type == TML_TOKEN_EOF) {
		set_parse_error(data, "Expected closing bracket on list");
	}

	if (token_out)
		*token_out = token;

	return root_node;
}


/* --------------- NODE ITERATION FUNCTIONS -------------------- */

static struct tml_node read_node(char *buff, char *ptr)
{
	struct tml_node node;
	node.buff = buff;

	if (((unsigned char*)ptr)[0] == FULL_NODE_DATA_FLAG) {
		/* read full node links */
		node.first_child = get_node_child(ptr);
		node.next_sibling = get_node_sibling(ptr);
		node.value = &ptr[NODE_LINK_DATA_SIZE];
	}
	else {
		/* read packed node links */
		node.first_child = 0;

		if (ptr[0] == 0)
			node.next_sibling = 0;
		else
			node.next_sibling = (ptr - buff) + 2 + ((unsigned char*)ptr)[0];

		node.value = &ptr[1];
	}

	return node;
}

struct tml_node tml_next_sibling(const struct tml_node *node)
{
	if (node->next_sibling)
		return read_node(node->buff, node->buff + node->next_sibling);
	else
		return NULL_NODE;
}

struct tml_node tml_first_child(const struct tml_node *node)
{
	if (node->first_child)
		return read_node(node->buff, node->buff + node->first_child);
	else
		return NULL_NODE;
}

int tml_child_count(const struct tml_node *node)
{
	int count = 0;
	struct tml_node cnode = tml_first_child(node);
	while (!tml_is_null(&cnode)) {
		count++;
		cnode = tml_next_sibling(&cnode);
	}
	return count;
}

struct tml_node tml_child_at_index(const struct tml_node *node, int child_index)
{
	int count = 0;
	struct tml_node cnode = tml_first_child(node);
	while (!tml_is_null(&cnode)) {
		if (count == child_index)
			return cnode;
		count++;
		cnode = tml_next_sibling(&cnode);
	}
	return NULL_NODE;
}


/* --------------------------------- UTILITY FUNCTIONS (CONVERSION) -------------------------------- */

static char *write_node_to_string(const struct tml_node *node, char *dest_str, char *dest_end, bool write_brackets)
{
	if (dest_str >= dest_end-1)
		return dest_str;

	if (!tml_has_children(node)) {
		char *value;
		size_t nodelen;

		if (!tml_is_list(node)) {
			value = node->value;
			nodelen = strlen(value);
		}
		else {
			if (write_brackets) {
				value = "[]";
				nodelen = 2;
			}
			else {
				return dest_str;
			}
		}

		size_t dest_str_size = dest_end - dest_str - 1;
		if (nodelen > dest_str_size)
			nodelen = dest_str_size;

		memcpy(dest_str, value, sizeof(char) * nodelen);

		dest_str += nodelen;
		return dest_str;
	}
	else {
		struct tml_node s_node = tml_first_child(node);
			
		if (write_brackets) {
			if (dest_str >= dest_end-1)
				return dest_str;
			*dest_str++ = '[';
		}

		for (;;) {
			dest_str = write_node_to_string(&s_node, dest_str, dest_end, write_brackets);

			s_node = tml_next_sibling(&s_node);
			if (tml_is_null(&s_node))
				break;

			if (dest_str >= dest_end-1)
				return dest_str;
			*dest_str++ = ' ';
		}

		if (write_brackets) {
			if (dest_str >= dest_end-1)
				return dest_str;
			*dest_str++ = ']';
		}

		return dest_str;
	}
}

size_t tml_node_to_string(const struct tml_node *node, char *dest_str, size_t dest_str_size)
{
	char *str = write_node_to_string(node, dest_str, dest_str + dest_str_size, false);
	size_t result_size = str - dest_str;

	if (result_size < dest_str_size) {
		*str = '\0';
		return result_size;
	}
	else {
		return 0; /* error - this shouldn't happen */
	}
}

size_t tml_node_to_markup_string(const struct tml_node *node, char *dest_str, size_t dest_str_size)
{
	char *str = write_node_to_string(node, dest_str, dest_str + dest_str_size, true);
	size_t result_size = str - dest_str;

	if (result_size < dest_str_size) {
		*str = '\0';
		return result_size;
	}
	else {
		return 0; /* error - this shouldn't happen */
	}
}

double tml_node_to_double(const struct tml_node *node)
{
	if (node->value && node->value[0] != 0) {
		return atof(node->value);
	}
	else return 0;
}

float tml_node_to_float(const struct tml_node *node)
{
	return tml_node_to_double(node);
}

int tml_node_to_int(const struct tml_node *node)
{
	if (node->value && node->value[0] != 0) {
		return atoi(node->value);
	}
	else return 0;
}

int tml_node_to_float_array(const struct tml_node *node, float *array, int array_size)
{
	struct tml_node n = tml_first_child(node);
	int count = 0;

	while (!tml_is_null(&n)) {
		if (count >= array_size) break;
		array[count++] = tml_node_to_float(&n);
		n = tml_next_sibling(&n);
	}

	return count;
}

int tml_node_to_double_array(const struct tml_node *node, double *array, int array_size)
{
	struct tml_node n = tml_first_child(node);
	int count = 0;

	while (!tml_is_null(&n)) {
		if (count >= array_size) break;
		array[count++] = tml_node_to_double(&n);
		n = tml_next_sibling(&n);
	}

	return count;
}

int tml_node_to_int_array(const struct tml_node *node, int *array, int array_size)
{
	struct tml_node n = tml_first_child(node);
	int count = 0;

	while (!tml_is_null(&n)) {
		if (count >= array_size) break;
		array[count++] = tml_node_to_int(&n);
		n = tml_next_sibling(&n);
	}

	return count;
}


/* --------------- UTILITY FUNCTIONS (COMPARISON / PATTERN MATCHING AND SEARCH) -------------------- */

static enum TML_WILDCARD check_wildcard(char *value)
{
	/* special string with ASCII character #1 and #2 are \? and \* wildcards */
	if (value[0] == '\0')
		return TML_NO_WILDCARD;
	if (value[1] == '\0' && (value[0] == 1 || value[0] == 2))
		return (enum TML_WILDCARD)value[0];
	else
		return TML_NO_WILDCARD;
}

bool tml_compare_nodes(const struct tml_node *candidate, const struct tml_node *pattern)
{
	if (!tml_is_list(pattern)) {
		/* expecting a "word" leaf node */
		if (tml_is_list(candidate)) return false;
		else return (strcmp(candidate->value, pattern->value) == 0);
	}
	else {
		struct tml_node p_child, c_child;
		enum TML_WILDCARD wild;

		/* at this point, we're expecting a list of zero or more items */
		if (!tml_is_list(candidate)) {
			return false;
		}

		/* if the pattern is an empty list [], then expect the same of the candidate */
		if (!tml_has_children(pattern)) {
			return !tml_has_children(candidate);
		}

		/* at this point, we're expecting a list of one or more items, so compare each element */

		/* if the pattern list starts with a \* wildcard, match anything, even an empty candidate list */
		p_child = tml_first_child(pattern);
		wild = check_wildcard(p_child.value);
		if (wild == TML_WILD_ANY)
			return true;
		c_child = tml_first_child(candidate);

		while (!tml_is_null(&c_child) && !tml_is_null(&p_child)) {
			/* the \? wildcard will match any single node, otherwise the node must be compared */
			if (wild != TML_WILD_ONE) {
				if (!tml_compare_nodes(&c_child, &p_child))
					return false;
			}

			/* a following \* wildcard matches the remainder of the list, regardless of what it is */
			p_child = tml_next_sibling(&p_child);
			wild = check_wildcard(p_child.value);
			if (wild == TML_WILD_ANY)
				return true;

			c_child = tml_next_sibling(&c_child);
		}

		/* if the candidate or pattern ran out of nodes before the other, they don't match */
		if (!tml_is_null(&c_child) || !tml_is_null(&p_child)) {
			return false;
		}

		/* everything seems to match */
		return true;
	}
}

struct tml_node tml_find_first_child(const struct tml_node *node, const struct tml_node *pattern)
{
	struct tml_node child = tml_first_child(node);

	while (!tml_is_null(&child)) {
		if (tml_compare_nodes(&child, pattern))
			return child;
		child = tml_next_sibling(&child);
	}

	return NULL_NODE;
}

struct tml_node tml_find_next_sibling(const struct tml_node *node, const struct tml_node *pattern)
{
	struct tml_node sib = tml_next_sibling(node);

	while (!tml_is_null(&sib)) {
		if (tml_compare_nodes(&sib, pattern))
			return sib;
		sib = tml_next_sibling(&sib);
	}

	return NULL_NODE;
}




