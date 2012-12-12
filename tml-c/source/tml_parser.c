#include "tml_parser.h"
#include "tml_tokenizer.h"

/*
 * Data is parsed and inserted into memory using a packed format to reduce the 
 * large overhead that would otherwise be incurred by the tree linkings (~24 bytes per node).
 * This format compresses that down to only 1 byte overhead per node for all sibling
 * leaf nodes.
 *
 * A node begins with the pointer data, followed by a null terminated value string IF
 * the pointer data indicates that this node is a leaf node.
 *
 * The pointer data is variable length, and has two forms:
 *
 * 1) One form is a single byte with value 0-254. In this case this value represents 
 * the relative offset to skip within the parsed data buffer to find the next subling node,
 * and also indicates that this is a leaf node (no child node pointer).
 *
 * 2) If the first byte is 255 then the next following bytes are a next_sibling
 * absolute offset followed by a first_child absolute offset.
 *
 * Because of the way the | nesting operator works (modifying the nesting depth of previously
 * parsed stuff), the parser needs to be able to insert an intermediate parent node sometimes.
 * To make this possible without shifting around memory, the parsing mechanism can just
 * enforce that all nodes following an open paren don't use the compressed form pointer data.
 */

void grow_buffer(struct tml_data *data)
{
	data->buff_allocated *= 2;
	data->buff = realloc(data->buff, data->buff_allocated);
}

void shrink_buffer(struct tml_data *data)
{
	data->buff_allocated = data->buff_size;
	data->buff = realloc(data->buff, data->buff_allocated);
}

void parse_root(struct tml_data *data, struct tml_stream *tokens);

struct tml_data *tml_parse_memory(char *ibuff, size_t ibuff_size)
{
	struct tml_data *data = malloc(sizeof(*data));
	memset(data, 0, sizeof(*data));

	data->error_msg = NULL;

	data->buff_size = 0;
	data->buff_allocated = buff_size * 2;
	data->buff = malloc(data->buff_allocated);

	struct tml_stream tokens = tml_stream_open(ibuff, ibuff_size);

	parse_node(data, &tokens);

	tml_stream_close(&tokens);
}

void tml_free_data(struct tml_data *data)
{
	free(data->buff);
	free(data);
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


void write_leaf(struct tml_data *data, char *str, int str_len)
{
}

void write_full_nonleaf(struct tml_data *data, size_t sibling_offset, size_t child_offset)
{
}

void write_packed_nonleaf(struct tml_data *data, int sibling_offset)
{
}


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

	parse_list(data, tokens);

	token = tml_stream_pop(tokens);

	if (token.type != TML_TOKEN_EOF) {
		set_parse_error(data, "Expected end of file - only one root node is allowed");
		return;
	}

	return;
}


void parse_list(struct tml_data *data, struct tml_stream *tokens)
{
	struct tml_token token = tml_stream_pop(tokens);

}




