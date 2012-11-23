#include "../source/tml_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tml_stream *create_stream(const char *text)
{
	struct tml_stream *stream = malloc(sizeof(*stream));

	size_t size = strlen(text);
	char *data = malloc(size);
	memcpy(data, text, size);
	
	*stream = tml_stream_open(data, size);

	return stream;
}

void destroy_stream(struct tml_stream *stream)
{
	char *data = stream->data;

	tml_stream_close(stream);
	free(stream);

	free(data);
}

static const char *tml_token_type_strings[] = { " ||EOF", "[", "]", "|", "" };

void print_token(char *dest, struct tml_token token)
{
	char buff0[1024];

	if (token.value) {
		char buff[1024];
		memcpy(buff, token.value, token.value_size);
		buff[token.value_size] = '\0';

		sprintf(buff0, "%s%s ", tml_token_type_strings[token.type], buff);
	}
	else {
		sprintf(buff0, "%s", tml_token_type_strings[token.type]);
	}

	strcat(dest, buff0);
}

void print_stream(char *dest, struct tml_stream *stream)
{
	struct tml_token token;

	dest[0] = '\0';

	token = tml_stream_pop(stream);

	while (token.type != TML_TOKEN_EOF) {
		print_token(dest, token);
		token = tml_stream_pop(stream);
	}
	print_token(dest, token);
}


int parse_and_redisplay_test(char *str_to_parse, char *str_to_verify)
{
	int ret = 0;
	char buff[2048];
	struct tml_stream *stream = create_stream(str_to_parse);

	print_stream(buff, stream);
	ret = strcmp(buff, str_to_verify);

	if (ret != 0) printf("%s\n", buff);
	destroy_stream(stream);
	return ret;
}


void test_assert(int i, int r)
{
	printf("#%d ", i);
	if (r != 0) printf("FAIL\n");
	else printf("PASS\n");
}


int main(void)
{
	test_assert( 1, 
		parse_and_redisplay_test("[ [a|] || this is a comment\n b c |\n 1 2 3 ]", "[[a |]b c |1 2 3 ] ||EOF") 
	);

	test_assert( 2, 
		parse_and_redisplay_test("[|[|[|[|[|!@#]]]]]", "[|[|[|[|[|!@# ]]]]] ||EOF") 
	);

	test_assert( 3, 
		parse_and_redisplay_test("[|right\\[ stuff]", "[|right[ stuff ] ||EOF") 
	);

	test_assert( 4, 
		parse_and_redisplay_test("[left stuff|]", "[left stuff |] ||EOF") 
	);

	test_assert( 5, 
		parse_and_redisplay_test("[a b c|1 2 3]", "[a b c |1 2 3 ] ||EOF") 
	);

	test_assert( 6, 
		parse_and_redisplay_test("[[", "[[ ||EOF") 
	);

	test_assert( 7, 
		parse_and_redisplay_test("[hello", "[hello  ||EOF") 
	);

	test_assert( 8, 
		parse_and_redisplay_test("\\\\", "\\  ||EOF") 
	);

	test_assert( 9, 
		parse_and_redisplay_test("\\", "  ||EOF") 
	);

	test_assert( 10, 
		parse_and_redisplay_test("[  ]", "[] ||EOF") 
	);

	return 0;
}
