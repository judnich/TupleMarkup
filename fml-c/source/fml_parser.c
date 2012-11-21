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

struct fml_stream fml_stream_open(void *data, size_t data_size)
{
	struct fml_stream stream;

	if (data == NULL || data_size == 0) {
		stream_memzero(&stream);
		return stream;
	}

	stream_memzero(&stream);
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

inline int peek_char(struct fml_stream *stream)
{
	if (stream->index < stream->data_size)
		return stream->data[stream->index];
	else
		return -1;
}

inline void next_char(struct fml_stream *stream)
{
	stream->index++;
}

struct fml_token get_error_token(const char *message, size_t file_offset)
{
	struct fml_token token;
	token.type = FML_TOKEN_ERROR;
	token.value = message;
	token.file_offset = file_offset;
	return token;
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

void skip_to_next_line(struct fml_stream *stream);
char *parse_word_item(struct fml_stream *stream);

struct fml_token fml_stream_pop(struct fml_stream *stream)
{
	if (stream->data == NULL || stream->data_size == 0) {
		return get_error_token("Invalid FML stream (no data)", 0);
	}

	struct fml_token token;
	token.value = NULL;
	token.file_offset = stream->index;

	for (;;) {
		int ch = peek_char(stream);
		next_char(stream);

		if (isspace(ch)) {
			continue;
		}
		else if (ch == '[') {
			token.type = FML_TOKEN_OPEN;
			return token;
		}
		else if (ch == ']') {
			token.type = FML_TOKEN_CLOSE;
			return token;
		}
		else if (ch == '|') {
			if (peek_char(stream) == '|') {
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
			token.value = parse_word_item(stream);
			token.type = FML_TOKEN_ITEM;
			return token;
		}
	}

	return get_error_token("Internal parser error", stream->index);
}

void skip_to_next_line(struct fml_stream *stream)
{
	for (;;) {
		int ch = peek_char(stream);
		next_char(stream);

		if (ch == '\n' || ch == '\r' || ch == -1)
			return;
	}
}

char *parse_word_item(struct fml_stream *stream)
{
	char *word_start = &stream->data[stream->index];
	char *p = word_start;
	bool shift_necessary = false;

	// scan the word, collapsing escape codes in-place if necessary
	int ch = peek_char(stream);
	while (!isspace(ch) && ch != -1 && ch != '|' && ch != '[' && ch != ']') {
		if (ch == '\\') {
			// substitute 2-character escape code with the character it represents
			next_char(stream);
			ch = peek_char(stream);
			if (ch == -1) break;
			*p = translate_escape_code(ch);
			shift_necessary = true;
		}
		else if (shift_necessary) {
			// shift regular character
			*p = (ch = peek_char(stream));
		}

		p++;
		next_char(stream);
		ch = peek_char(stream);
	}

	// null-terminate the resulting string in-place
	*p = '\0';

	// return a reference to the data, because we assume the original data
	return word_start;
}


