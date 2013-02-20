/* 
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence. 
 */

#include "tml_tokenizer.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifdef _MSC_VER
#define INLINE __inline
#else
#define INLINE __inline__
#endif
 

void stream_memzero(struct tml_stream *stream)
{
	memset(stream, 0, sizeof(*stream));
}

struct tml_stream tml_stream_open(char *data, size_t data_size)
{
	struct tml_stream stream;
	stream_memzero(&stream);

	if (data == NULL || data_size == 0) {
		return stream;
	}

	stream.data = data;
	stream.data_size = data_size;
	stream.index = 0;

	return stream;
}

void tml_stream_close(struct tml_stream *stream)
{
	if (stream)
		stream_memzero(stream);
}

static INLINE int peek_char(struct tml_stream *stream)
{
	return (stream->index < stream->data_size) ? 
			stream->data[stream->index] : -1 ;
}

static INLINE void next_char(struct tml_stream *stream)
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
		/* special wildcard codes for pattern-match strings */
		case '?': return (char)TML_WILD_ONE; 
		case '*': return (char)TML_WILD_ANY;
	default:
		return code;
	}
}

struct tml_token parse_token(struct tml_stream *stream);
void skip_to_next_line(struct tml_stream *stream);
void parse_word_item(struct tml_stream *stream, struct tml_token *token);

struct tml_token tml_stream_pop(struct tml_stream *stream)
{
	struct tml_token token;
	token.value = NULL;
	token.value_size = 0;

	for (;;) {
		int ch = peek_char(stream);

		if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
			next_char(stream);
			continue;
		}

		token.offset = stream->index;

		if (ch == TML_OPEN_CHAR) {
			next_char(stream);
			token.type = TML_TOKEN_OPEN;
			return token;
		}
		else if (ch == TML_CLOSE_CHAR) {
			next_char(stream);
			token.type = TML_TOKEN_CLOSE;
			return token;
		}
		else if (ch == TML_DIVIDER_CHAR) {
			next_char(stream);
			if (peek_char(stream) == TML_DIVIDER_CHAR) {
				skip_to_next_line(stream);
				continue;
			}
			else {
				token.type = TML_TOKEN_DIVIDER;
				return token;
			}
		}
		else if (ch == -1) {
			token.type = TML_TOKEN_EOF;
			return token;
		}
		else {
			parse_word_item(stream, &token);
			return token;
		}
	}
}

void skip_to_next_line(struct tml_stream *stream)
{
	for (;;) {
		int ch = peek_char(stream);
		next_char(stream);

		if (ch == '\n' || ch == '\r' || ch == -1)
			return;
	}
}

/* This function is a bit messy unfortunately since it does efficient in-place parsing of "words"
 * with escape codes. When escape codes are encountered, it collapses them to the actual character 
 * value in-place in memory. The token data generated from this operation points to the word within
 * the stream data memory. */
void parse_escaped_word_item(struct tml_stream *stream, struct tml_token *token)
{
	char *word_start = &stream->data[stream->index];
	char *p = word_start;
	bool shift_necessary = false;

	/* scan the word, collapsing escape codes in-place if necessary */
	int ch = peek_char(stream);
	while (ch != ' ' && ch != '\t' && ch != -1 && 
		ch != TML_DIVIDER_CHAR && ch != TML_OPEN_CHAR && ch != TML_CLOSE_CHAR)
	{
		if (ch == TML_ESCAPE_CHAR) {
			/* substitute 2-character escape code with the character it represents */
			next_char(stream);
			ch = peek_char(stream);
			if (ch == -1) break;
			*p = translate_escape_code(ch);
			shift_necessary = true;
		}
		else if (shift_necessary) {
			/* shift character to the left collapsed position */
			*p = (ch = peek_char(stream));
		}

		/* go on to the next potential character */
		p++;
		next_char(stream);
		ch = peek_char(stream);
	}

	/* return a reference to the data slice */
	token->type = TML_TOKEN_ITEM;
	token->value = word_start;
	token->value_size = (p - word_start);
}

/* This function reads in a word by quickly skimming to the end. This only works if it doesn't use escape
 * codes - if it bumps into one, it reverts to parse_escaped_word_item() to do the job. */
void parse_word_item(struct tml_stream *stream, struct tml_token *token)
{
	char *word_start = &stream->data[stream->index], *data_end = &stream->data[stream->data_size];
	char *p = word_start;

	/* Scan up to the end of the word.
	 * Note that some (ugly) manual loop unrolling is performed here.
	 * This does improve performance by a noticeable amount. */
	#define CONDITION (ch != ' ' && ch != '\t' && ch != TML_ESCAPE_CHAR &&\
			ch != TML_DIVIDER_CHAR && ch != TML_OPEN_CHAR && ch != TML_CLOSE_CHAR)
	#define NEXT_CHAR ++p; ch = *p;
	#define COND_NEXT_CHAR if (CONDITION) { NEXT_CHAR } else { break; }
	char ch = *p;
	while ((p < data_end-8) && CONDITION) {
		NEXT_CHAR  COND_NEXT_CHAR  COND_NEXT_CHAR  COND_NEXT_CHAR
		COND_NEXT_CHAR  COND_NEXT_CHAR  COND_NEXT_CHAR  COND_NEXT_CHAR
	}
	while ((p < data_end) && CONDITION) {
		NEXT_CHAR
	}

	/* if encountered an escape code, cancel this function's work, and use another more complex (slower) function */
	if (ch == TML_ESCAPE_CHAR) {
		parse_escaped_word_item(stream, token);
		return;
	}

	/* return a reference to the data slice */
	token->type = TML_TOKEN_ITEM;
	token->value = word_start;
	token->value_size = (p - word_start);

	stream->index += token->value_size;
}



