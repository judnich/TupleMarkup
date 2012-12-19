#include "tml_parser.h"
#include "tml_tokenizer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*
 * Data is parsed and inserted into memory using a packed format to reduce the 
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


void set_parse_error(struct tml_data *data, const char *error_msg);
void parse_root(struct tml_data *data, struct tml_stream *tokens);
size_t parse_list_node(struct tml_data *data, struct tml_stream *tokens, bool process_divider, struct tml_token *token_out);


#define FULL_NODE_DATA_FLAG 0xFF
const int NODE_LINK_DATA_SIZE = sizeof(char) + sizeof(tml_offset_t)*2;


void grow_buffer_if_needed(struct tml_data *data, size_t new_size)
{
	if (new_size >= TML_PARSER_MAX_DATA_SIZE) {
		set_parse_error(data, 
			"TML data file is too large, parsed data structures exceeded TML_PARSER_MAX_DATA_SIZE.");
	}

	if (new_size > data->buff_allocated) {
		data->buff_allocated *= 2;
		data->buff = realloc(data->buff, data->buff_allocated);
	}
}

void shrink_buffer(struct tml_data *data)
{
	data->buff_allocated = data->buff_index;
	data->buff = realloc(data->buff, data->buff_allocated);
}


struct tml_data *tml_parse_memory(char *ibuff, size_t ibuff_size)
{
	struct tml_data *data = malloc(sizeof(*data));
	memset(data, 0, sizeof(*data));

	data->error_msg = NULL;
	data->buff_index = 0;
	data->buff_allocated = ibuff_size * 2;
	data->buff = malloc(data->buff_allocated);

	struct tml_stream tokens = tml_stream_open(ibuff, ibuff_size);
	parse_root(data, &tokens);
	tml_stream_close(&tokens);

	shrink_buffer(data);

	data->root_node.buff = data->buff;

	return data;
}

struct tml_data *tml_parse_string(char *str)
{
	size_t len = strlen(str);
	return tml_parse_memory(str, len);
}

struct tml_data *tml_parse_file(char *filename)
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

	struct tml_data *data = tml_parse_memory(buff, fsize);

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

void set_parse_error(struct tml_data *data, const char *error_msg)
{
	if (data->error_msg == NULL)
		data->error_msg = error_msg;
}

const char *tml_parse_error(struct tml_data *data)
{
	return data->error_msg;
}

struct tml_node tml_data_root(struct tml_data *data)
{
	return data->root_node;
}


void write_packed_node(struct tml_data *data, const char *str, int str_len, int sibling_offset)
{
	size_t index = data->buff_index;

	grow_buffer_if_needed(data, 
		index + sizeof(char) + (str_len + 1) * sizeof(char));

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

size_t write_node(struct tml_data *data, const char *str, int str_len)
{
	size_t index = data->buff_index;

	grow_buffer_if_needed(data, 
		index + NODE_LINK_DATA_SIZE + (str_len + 1) * sizeof(char));

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

__inline__ void update_node_child(char *node_ptr, size_t first_child)
{
	tml_offset_t *fc = (tml_offset_t*)(node_ptr + 1);
	*fc = (tml_offset_t)first_child;
}

__inline__ void update_node_sibling(char *node_ptr, size_t next_sibling)
{
	tml_offset_t *ns = (tml_offset_t*)(node_ptr + 1 + sizeof(tml_offset_t));
	*ns = (tml_offset_t)next_sibling;
}

__inline__ size_t get_node_child(char *node_ptr)
{
	tml_offset_t *fc = (tml_offset_t*)(node_ptr + 1);
	return *fc;
}

__inline__ size_t get_node_sibling(char *node_ptr)
{
	tml_offset_t *ns = (tml_offset_t*)(node_ptr + 1 + sizeof(tml_offset_t));
	return *ns;
}


/* Parses "[...]" */
void parse_root(struct tml_data *data, struct tml_stream *tokens)
{
	struct tml_token token = tml_stream_pop(tokens);

	if (token.type != TML_TOKEN_OPEN) {
		if (token.type == TML_TOKEN_EOF)
			set_parse_error(data, "File contents is empty");
		else
			set_parse_error(data, "Expecting opening bracket at start of file");
		return;
	}

	size_t root_node_offset = parse_list_node(data, tokens, true, NULL);

	token = tml_stream_pop(tokens);

	if (token.type != TML_TOKEN_EOF) {
		set_parse_error(data, "Expected end of file after end of root node");
		return;
	}

	data->root_node.buff = data->buff;
	data->root_node.value = NULL;
	data->root_node.leaf_node = false;
	data->root_node.next_sibling = 0;
	data->root_node.first_child = get_node_child(&data->buff[root_node_offset]);

	return;
}


/* Parses "...]", a list where we assume that the opening brace has been read. 
 * After returning from this function, the stream will have read the closing brace. */
size_t parse_list_node(struct tml_data *data, struct tml_stream *tokens, bool process_divider, struct tml_token *token_out)
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
	}

	if (token.type == TML_TOKEN_EOF) {
		set_parse_error(data, "Expected closing bracket on list");
	}

	if (token_out)
		*token_out = token;

	return root_node;
}


/* --------------- NODE ITERATION FUNCTIONS -------------------- */

struct tml_node NULL_NODE = { 0, 0, 0, 0, 0 };

struct tml_node read_node(char *buff, char *ptr)
{
	struct tml_node node;
	node.buff = buff;

	if (((unsigned char*)ptr)[0] == FULL_NODE_DATA_FLAG) {
		/* read full node links */
		node.first_child = get_node_child(ptr);
		node.next_sibling = get_node_sibling(ptr);
		node.leaf_node = (node.first_child == 0);

		ptr += NODE_LINK_DATA_SIZE;
	}
	else {
		/* read packed node links */
		node.leaf_node = true;
		node.first_child = 0;
		if (ptr[0] == 0)
			node.next_sibling = 0;
		else
			node.next_sibling = (ptr - buff) + 2 + ((unsigned char*)ptr)[0];

		ptr += sizeof(char);
	}

	/* node value string follows link data */
	node.value = (char*)ptr;

	return node;
}

struct tml_node tml_next_sibling(struct tml_node *node)
{
	if (node->next_sibling)
		return read_node(node->buff, node->buff + node->next_sibling);
	else
		return NULL_NODE;
}

struct tml_node tml_first_child(struct tml_node *node)
{
	if (!node->leaf_node)
		return read_node(node->buff, node->buff + node->first_child);
	else
		return NULL_NODE;
}

int tml_child_count(struct tml_node *node)
{
	int count = 0;
	struct tml_node cnode = tml_first_child(node);
	while (!tml_is_node_null(&cnode)) {
		count++;
		cnode = tml_next_sibling(&cnode);
	}
	return count;
}

struct tml_node tml_child_at_index(struct tml_node *node, int child_index)
{
	int count = 0;
	struct tml_node cnode = tml_first_child(node);
	while (!tml_is_node_null(&cnode)) {
		if (count == child_index)
			return cnode;
		count++;
		cnode = tml_next_sibling(&cnode);
	}
	return NULL_NODE;
}

bool tml_is_node_null(struct tml_node *node)
{
	return node->buff == 0; //(node->next_sibling == 0 && node->first_child == 0 && node->value == 0);
}

bool tml_is_node_leaf(struct tml_node *node)
{
	return node->leaf_node;
}


/* ------------------------------------ UTILITY FUNCTIONS ------------------------------------------------ */

char *write_node_to_string(struct tml_node *node, char *dest_str, char *dest_end, bool write_brackets)
{
	if (dest_str >= dest_end-1)
		return dest_str;

	if (tml_is_node_leaf(node)) {
		char *value = node->value;

		size_t nodelen = strlen(value);
		if (nodelen == 0) {
			value = "[]";
			nodelen = 2;
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
			
		if (dest_str >= dest_end-1)
			return dest_str;
		if (write_brackets)
			*dest_str++ = '[';

		for (;;) {
			dest_str = write_node_to_string(&s_node, dest_str, dest_end, write_brackets);

			s_node = tml_next_sibling(&s_node);
			if (tml_is_node_null(&s_node))
				break;

			if (dest_str >= dest_end-1)
				return dest_str;
			*dest_str++ = ' ';
		}

		if (dest_str >= dest_end-1)
			return dest_str;
		if (write_brackets)
			*dest_str++ = ']';

		return dest_str;
	}
}

size_t tml_node_to_string(struct tml_node *node, char *dest_str, size_t dest_str_size)
{
	char *str = write_node_to_string(node, dest_str, dest_str + dest_str_size, false);
	size_t result_size = str - dest_str;

	if (result_size < dest_str_size) {
		*str = '\0';
		return result_size;
	}
	else {
		return 0; // error - this shouldn't happen
	}
}

size_t tml_node_to_markup_string(struct tml_node *node, char *dest_str, size_t dest_str_size)
{
	char *str = write_node_to_string(node, dest_str, dest_str + dest_str_size, true);
	size_t result_size = str - dest_str;

	if (result_size < dest_str_size) {
		*str = '\0';
		return result_size;
	}
	else {
		return 0; // error - this shouldn't happen
	}
}



