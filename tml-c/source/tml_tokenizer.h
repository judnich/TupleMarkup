/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This lexer runs entirely in-place with no heap allocations whatosever.
 *
 * A memory buffer is given to the parser. The parser then reads through it on demand
 * and may modify it (e.g. collapsing escape codes). Returned token.value's are pointers
 * into the original buffer memory, as no copying occurs.
 *
 * Closing a token stream does not invalidate returned tokens because you retain ownership
 * of the original data buffer into which token.value's point. However this also means that
 * as soon as you free your original buffer, all token.value pointers become invalid.
 *
 * The recommended data ownership / lifecycle is as follows:
 *
 * 1. Allocate/load TML text data into a buffer.
 * 2. Create/open a tml_stream and read out tokens.
 * 3. When EOF token is returned, close/discard the tml_stream.
 * 4. Continue to use the token.value data as long as you like.
 * 5. Free the buffer from step 1, thus invalidating all token data.

 * If you really need to free the data buffer as soon as you close the token stream,
 * then just make your own local copies of token.value with memcpy first. */

#pragma once
#ifndef _TML_TOKENIZER_H__
#define _TML_TOKENIZER_H__

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

	const char *value; /* IMPORTANT: value is NOT a null-terminated C string. */
	size_t value_size;

	size_t offset;
};

/* Call tml_stream_open() to setup a stream to start reading tokens from. Pass this function
 * a memory buffer containing the ASCII TML text data to parse. You retain ownership of the 
 * buffer. You can think of struct tml_stream as an object containing a queue of tokens. */ 
struct tml_stream tml_stream_open(char *data, size_t data_size);

/* Always call tml_stream_close() once you're finished with a tml_stream object. After you close
 * the stream you may then free the data buffer you originally gave the stream whwnever you
 * like, but do not free it before then. Note that deleting the data buffer will invalidate
 * any tokens returned by tml_stream_pop(). */
void tml_stream_close(struct tml_stream *stream);

/* Repeatedly call tml_stream_pop() to dequeue tokens from the stream. When the end of 
 * the data is reached, TML_TOKEN_EOF tokens will be returned. IMPORTANT: Any tokens
 * returned from here will have their data *invalidated* if you free the data buffer
 * from which they were extracted, because token data is returned as pointers into this
 * original data buffer.  */
struct tml_token tml_stream_pop(struct tml_stream *stream);


#endif
