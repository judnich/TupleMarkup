#include "tml_parser.h"
#include "tml_tokenizer.h"

#include <stdlib.h>
#include <string.h>

/*
 * Data is parsed and inserted into memory using a packed format to reduce the 
 * overhead that would otherwise be incurred by the tree linkings (~9 bytes per node).
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
struct node_link_data *parse_list_node(struct tml_data *data, struct tml_stream *tokens);


#define FULL_NODE_DATA_FLAG 0xFF

struct node_link_data
{
 	unsigned char flag; /* this will always be FULL_NODE_DATA_FLAG */
 	tml_offset_t next_sibling;
	tml_offset_t first_child;
};


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


struct node_link_data *write_node(struct tml_data *data, const char *str, int str_len)
{
	size_t index = data->buff_index;

	grow_buffer_if_needed(data, 
		index + sizeof(struct node_link_data) + (str_len + 1) * sizeof(char));

	/* write node link data */
	struct node_link_data *ptr = (struct node_link_data*)data->buff;
	ptr->flag = 0xFF; // code for a full node metadata
	ptr->next_sibling = 0;
	ptr->first_child = 0;
	index += sizeof(struct node_link_data);

	/* copy string contents */
	if (str_len > 0) {
		memcpy(data->buff + index, str, str_len * sizeof(char));
		index += str_len * sizeof(char);
	}

	/* null terminate string */
	data->buff[index] = '\0';
	index += sizeof(char);

	data->buff_index = index;
	return ptr;
}

void write_packed_node(struct tml_data *data, const char *str, int str_len, int sibling_offset)
{
	size_t index = data->buff_index;

	/* write sibling offset byte */
	data->buff[index] = (unsigned char)sibling_offset;
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

	parse_list_node(data, tokens);

	token = tml_stream_pop(tokens);

	if (token.type != TML_TOKEN_EOF) {
		set_parse_error(data, "Expected end of file after end of root node");
		return;
	}

	return;
}


/* Parses "...]", a list where we assume that the opening brace has been read. 
 * After returning from this function, the stream will have read the closing brace. */
struct node_link_data *parse_list_node(struct tml_data *data, struct tml_stream *tokens)
{
	/* this is the container node for the list contents */
	struct node_link_data *root_node = write_node(data, NULL, 0);

	struct tml_token token, last_token;
	bool peeked = false;

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
			if (!root_node->first_child)
				root_node->first_child = data->buff_index;

			/* peek ahead to see if there is a next sibling */
			last_token = token;
			token = tml_stream_pop(tokens);
			peeked = true;

			if (token.type == TML_TOKEN_CLOSE || token.type == TML_TOKEN_EOF) {
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
					struct node_link_data *n = write_node(data, last_token.value, last_token.value_size);
					n->next_sibling = data->buff_index;
				}
			}
		}
		else if (token.type == TML_TOKEN_OPEN) {
			/* record the first child under this list */
			if (!root_node->first_child)
				root_node->first_child = data->buff_index;

			/* recurse into a new list item */
			struct node_link_data *list_node = parse_list_node(data, tokens);

			/* peek ahead to see if there is a next sibling */
			last_token = token;
			token = tml_stream_pop(tokens);
			peeked = true;

			/* set the list's next node link the current writing position if there are more list items */
			if (token.type == TML_TOKEN_CLOSE || token.type == TML_TOKEN_EOF) {
				list_node->next_sibling = 0;
			}
			else {
				list_node->next_sibling = data->buff_index;
			}
		}
		else if (token.type == TML_TOKEN_DIVIDER) {
			/* todo - 
			 * the idea for how this works in a single pass is, when a divider is encountered, the root_node
			 * first_child* pointer is "rewired" to point to a new intermediate list node created here. this
			 * intermediate node then is created to point back to the "original" first child node, creating the 
			 * desired double nesting. then, another nested list node is created for the upcoming items after the
			 * bar (since they're also nested) and each following bar creates a new intermediate node.
			 */
		}
		else if (token.type == TML_TOKEN_CLOSE || token.type == TML_TOKEN_EOF) {
			if (token.type == TML_TOKEN_EOF) {
				set_parse_error(data, "Expected closing bracket on list");
			}
			break;
		}
	}

	return root_node;
}




