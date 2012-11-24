/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This parser runs entirely in-place with no heap allocations whatosever.
 * A memory buffer is given to the parser. The parser then reads through it on demand
 * and may modify it (e.g. collapsing escape codes). Returned tokens reference memory
 * within the original buffer, as no copying occurs. You retain allocation ownership 
 * of the data though the parser may modify it in the process of parsing it. Free
 * it yourself when you're done however you like. This way, you can close an tml_stream 
 * without invalidating any token values extracted from it.
 */

#pragma once
#ifndef _TML_PARSER_H__
#define _TML_PARSER_H__

#include <ctype.h>

struct tml_stream
{
	char *data;
	size_t data_size;
	size_t index;
};

enum TML_TOKEN_TYPE
{
	TML_TOKEN_EOF, TML_TOKEN_OPEN, TML_TOKEN_CLOSE, TML_TOKEN_DIVIDER, TML_TOKEN_ITEM
};

struct tml_token
{
	enum TML_TOKEN_TYPE type;
	const char *value;
	size_t value_size;
	size_t offset;
};

struct tml_stream tml_stream_open(char *data, size_t data_size);
void tml_stream_close(struct tml_stream *stream);

struct tml_token tml_stream_pop(struct tml_stream *stream);


#endif
