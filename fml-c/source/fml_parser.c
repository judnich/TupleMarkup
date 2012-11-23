// Copyright (C) 2012 John Judnich
// Released as open-source under The MIT Licence.

#include "fml_parser.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

void stream_memzero(struct fml_stream *stream)
{
	memset(stream, 0, sizeof(*stream));
}

struct fml_stream fml_stream_open(char *data, size_t data_size)
{
	struct fml_stream stream;
	stream_memzero(&stream);

	if (data == NULL || data_size == 0) {
		return stream;
	}

	stream.data = data;
	stream.data_size = data_size;
	stream.index = 0;

	return stream;
}

void fml_stream_close(struct fml_stream *stream)
{
	if (!stream) return;
	stream_memzero(stream);
}

inline int peek_char(struct fml_stream *stream, size_t offset)
{
	if (stream->index + offset < stream->data_size)
		return stream->data[stream->index + offset];
	else
		return -1;
}

inline void next_char(struct fml_stream *stream)
{
	stream->index++;
}

char translate_escape_code(char code)
{
	switch (code) {
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 's': return ' ';
	default:
		return code;
	}
}

struct fml_token parse_token(struct fml_stream *stream);
void skip_to_next_line(struct fml_stream *stream);
void parse_word_item(struct fml_stream *stream, struct fml_token *token);

struct fml_token fml_stream_pop(struct fml_stream *stream)
{
	struct fml_token token;
	token.value = NULL;
	token.value_size = 0;

	for (;;) {
		int ch = peek_char(stream, 0);

		if (isspace(ch)) {
			next_char(stream);
			continue;
		}

		token.offset = stream->index;

		if (ch == '[') {
			next_char(stream);
			token.type = FML_TOKEN_OPEN;
			return token;
		}
		else if (ch == ']') {
			next_char(stream);
			token.type = FML_TOKEN_CLOSE;
			return token;
		}
		else if (ch == '|') {
			next_char(stream);
			if (peek_char(stream, 0) == '|') {
				skip_to_next_line(stream);
				continue;
			}
			else {
				token.type = FML_TOKEN_DIVIDER;
				return token;
			}
		}
		else if (ch == -1) {
			token.type = FML_TOKEN_EOF;
			return token;
		}
		else {
			parse_word_item(stream, &token);
			return token;
		}
	}
}

void skip_to_next_line(struct fml_stream *stream)
{
	for (;;) {
		int ch = peek_char(stream, 0);
		next_char(stream);

		if (ch == '\n' || ch == '\r' || ch == -1)
			return;
	}
}

void parse_word_item(struct fml_stream *stream, struct fml_token *token)
{
	char *word_start = &stream->data[stream->index];
	char *p = word_start;
	bool shift_necessary = false;

	// scan the word, collapsing escape codes in-place if necessary
	int ch = peek_char(stream, 0);
	while (!isspace(ch) && ch != -1 && ch != '|' && ch != '[' && ch != ']') {
		if (ch == '\\') {
			// substitute 2-character escape code with the character it represents
			next_char(stream);
			ch = peek_char(stream, 0);
			if (ch == -1) break;
			*p = translate_escape_code(ch);
			shift_necessary = true;
		}
		else if (shift_necessary) {
			// shift character to the left collapsed position
			*p = (ch = peek_char(stream, 0));
		}

		// go on to the next potential character
		p++;
		next_char(stream);
		ch = peek_char(stream, 0);
	}

	// return a reference to the data slice
	token->type = FML_TOKEN_ITEM;
	token->value = word_start;
	token->value_size = (p - word_start);
}


