#include "../source/tml_tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PASS_MSG "pass"
#define FAIL_MSG "[ F A I L ]"

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

int g_test_num = 0, g_pass_count = 0;

void test_parser(char *str_to_parse, char *str_to_verify)
{
	char buff[2048];
	struct tml_stream *stream = create_stream(str_to_parse);

	g_test_num++;
	printf("#%d ", g_test_num);

	print_stream(buff, stream);
	
	if (strcmp(buff, str_to_verify) == 0) {
		printf("%s\n", PASS_MSG);
		g_pass_count++;
	}
	else {
		printf("%s: Produced \"%s\". Expected \"%s\".\n", FAIL_MSG, buff, str_to_verify);
	}

	destroy_stream(stream);
	return;
}

void print_report()
{
	int pp = (int)(100 * (float)g_pass_count / (float)g_test_num);
	printf("\n - Tokenizer Test Suite: %d tests executed, %d passed (%d%%).\n\n", g_test_num, g_pass_count, pp);
}


int main(void)
{
	printf("\n==== TML Tokenizer Test Suite ====\n\n");

	test_parser("a b c", "a b c  ||EOF");
	test_parser("\\[", "[  ||EOF");
	test_parser("\\]", "]  ||EOF");
	test_parser("\\|", "|  ||EOF");
	test_parser("[ [a|] || this is a comment\n b c |\n 1 2 3 ]", "[[a |]b c |1 2 3 ] ||EOF");
	test_parser("[|[|[|[|[|!@#]]]]]", "[|[|[|[|[|!@# ]]]]] ||EOF");
	test_parser("[|right\\[ stuff]", "[|right[ stuff ] ||EOF");
	test_parser("[left stuff|]", "[left stuff |] ||EOF");
	test_parser("[a b c|1 2 3]", "[a b c |1 2 3 ] ||EOF");
	test_parser("[[", "[[ ||EOF");
	test_parser("[hello", "[hello  ||EOF");
	test_parser("\\\\", "\\  ||EOF");
	test_parser("\\", "  ||EOF");
	test_parser("[  ]", "[] ||EOF");

	print_report();

	return 0;
}
