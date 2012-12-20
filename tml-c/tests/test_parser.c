#include "../source/tml_parser.h"

#include <stdio.h>
#include <string.h>

int g_test_num = 0, g_pass_count = 0;

#define PASS_MSG "pass"
#define FAIL_MSG "[ F A I L ]"

void test_tml(char *source_string, char *expected_output, bool brackets)
{
	struct tml_data *data = tml_parse_string(source_string);
	struct tml_node root = tml_data_root(data);

	const char *err = tml_parse_error(data);

	g_test_num++;
	printf("#%d ", g_test_num);

	if (expected_output == NULL) {
		if (err) {
			printf("%s\n", PASS_MSG);
			g_pass_count++;
		}
		else
			printf("%s: Expected error.\n", FAIL_MSG);
		tml_free_data(data);
		return;
	}

	if (err) {
		printf("%s: Unexpected parse error: \"%s\"\n", FAIL_MSG, err);
		tml_free_data(data);
		return;
	}

	char buff[1024];
	if (brackets)
		tml_node_to_markup_string(&root, buff, 1024);
	else
		tml_node_to_string(&root, buff, 1024);

	if (strcmp(buff, expected_output) == 0) {
		printf("%s\n", PASS_MSG);
		g_pass_count++;
	}
	else {
		printf("%s: Produced \"%s\", expected \"%s\".\n", FAIL_MSG, buff, expected_output);
	}

	tml_free_data(data);	
}

/* This test makes sure the node to string function respects the buffer length limit.
 * This is very important to test for security reasons (don't want buffer overflows) so this
 * exhaustively tests every reasonable length limit for the test string and verifies that it
 * writes the correct output AND does not exceed (or under-ceed) the expected length. */
void test_node_to_string(const char *parse_str, const char *test_str, bool brackets)
{
	int test_str_len = strlen(test_str);

	struct tml_data *data = tml_parse_string(parse_str);
	struct tml_node root = tml_data_root(data);

	char buff[1024];
	int i, j;

	g_test_num++;
	printf("#%d ", g_test_num);

	for (i = 0; i < test_str_len * 2; i++) {
		size_t expected_size = i, size;
		if (expected_size > test_str_len) expected_size = test_str_len;

		memset(buff, 0, sizeof(buff));

		if (brackets)
			size = tml_node_to_markup_string(&root, buff, i+1);
		else
			size = tml_node_to_string(&root, buff, i+1);

		if (size != expected_size) {
			printf("%s: Node to string conversion %d claimed size %ld, expected %ld.\n", FAIL_MSG, i, size, expected_size);
			tml_free_data(data);
			return;
		}

		for (j = 0; j < expected_size; ++j) {
			if (buff[j] != test_str[j]) {
				printf("%s: Node to string test failed - invalid string produced.\n", FAIL_MSG);
				tml_free_data(data);
				return;
			}
		}
		if (buff[expected_size] != '\0') {
			printf("%s: Node to string test failed - invalid null termination string length.\n", FAIL_MSG);
			tml_free_data(data);
			return;
		}
	}

	printf("%s\n", PASS_MSG);
	g_pass_count++;

	tml_free_data(data);
}

void test_pattern_match(const char *candidate, const char *pattern, bool match)
{
	g_test_num++;
	printf("#%d ", g_test_num);

	struct tml_data *c_data = tml_parse_string(candidate);
	struct tml_data *p_data = tml_parse_string(pattern);

	bool actual_match = tml_compare_nodes(tml_data_root_ptr(c_data), tml_data_root_ptr(p_data));
	if (actual_match == match) {
		printf("%s\n", PASS_MSG);
		g_pass_count++;
	}
	else {
		printf("%s\n", FAIL_MSG);
	}

	tml_free_data(c_data);
	tml_free_data(p_data);
}

void print_report()
{
	int pp = (int)(100 * (float)g_pass_count / (float)g_test_num);
	printf("\n - Parser Test Suite: %d tests executed, %d passed (%d%%).\n\n", g_test_num, g_pass_count, pp);
}

int main(void)
{
	printf("\n==== TML Parser Test Suite ====\n\n");

	/* test errors */
	test_tml("", NULL, true);
	test_tml("must-begin-with-a-list", NULL, true);
	test_tml("[only one root] [node is allowed]", NULL, true);
	test_tml("only one root node is allowed", NULL, true);
	test_tml("[forgot to [close | my] bracket", NULL, true);
	test_tml("a b", NULL, true);
	test_tml("[", NULL, true);
	test_tml("[[]", NULL, true);
	test_tml("]", NULL, true);
	test_tml("|", NULL, true);
	test_tml("[|", NULL, true);
	test_tml("[|[a b]|", NULL, true);
	test_tml("[a b", NULL, true);

	/* test string conversion */
	test_tml("[]", "", false);
	test_tml("[test]", "test", false);
	test_tml("[this [is [a [test]]]]", "this is a test", false);

	test_tml("[bold | hello [italic | this] is a test]",
		"bold hello italic this is a test", false);

	/* test tml code parsing / generation */
	test_tml("[]", "[]", true);
	test_tml("[test]", "[test]", true);

	test_tml("[|]", "[[] []]", true);
	test_tml("[a|]", "[[a] []]", true);
	test_tml("[|a]", "[[] [a]]", true);
	test_tml("[| |]", "[[] [] []]", true);

	test_tml("[a b c]", "[a b c]", true);
	test_tml("[a [] b]", "[a [] b]", true);
	test_tml("[[[]]]", "[[[]]]", true);
	test_tml("[a b c | d e f]", "[[a b c] [d e f]]", true);
	test_tml("[a | b | c | d | e]", "[[a] [b] [c] [d] [e]]", true);
	
	test_tml("[bold | hello [italic | this] is a test]",
		"[[bold] [hello [[italic] [this]] is a test]]", true);

	test_node_to_string("[this [is [a [test]]]]", "[this [is [a [test]]]]", true);
	test_node_to_string("[[]]", "[[]]", true);
	test_node_to_string("[[] []]", "[[] []]", true);
	test_node_to_string("[[a]]", "[[a]]", true);
	test_node_to_string("[[a] [b]]", "[[a] [b]]", true);
	test_node_to_string("[a [b [c] d] e]", "[a [b [c] d] e]", true);

	test_node_to_string("[this [is [a [test]]]]", "this is a test", false);
	test_node_to_string("[[]]", "", false);
	test_node_to_string("[[] []]", "", false);
	test_node_to_string("[[a]]", "a", false);
	test_node_to_string("[[a] [b]]", "a b", false);
	test_node_to_string("[a [b [c] d] e]", "a b c d e", false);

	/* test pattern matching */
	test_pattern_match("[]", "[]", true);
	test_pattern_match("[|]", "[[][]]", true);
	test_pattern_match("[a b c]", "[a b c]", true);
	test_pattern_match("[a [b] c]", "[a [b] c]", true);

	test_pattern_match("[|]", "[]", false);
	test_pattern_match("[a b c]", "[a b c d]", false);
	test_pattern_match("[a b c d]", "[a b c]", false);
	test_pattern_match("[a b c]", "[c b a]", false);
	test_pattern_match("[a b c]", "[a b d]", false);
	test_pattern_match("[c a b]", "[d a b]", false);

	test_pattern_match("[]", "[\\*]", true);
	test_pattern_match("[a]", "[\\*]", true);
	test_pattern_match("[a b c]", "[\\*]", true);
	test_pattern_match("[[a] [b c]]", "[\\*]", true);

	test_pattern_match("[]", "[\\? \\*]", false);
	test_pattern_match("[a b]", "[\\? \\*]", true);
	test_pattern_match("[a b]", "[\\? \\? \\*]", true);
	test_pattern_match("[a b c]", "[\\? \\? \\*]", true);
	test_pattern_match("[a b c]", "[\\? \\*]", true);
	test_pattern_match("[a b c d]", "[\\? \\*]", true);
	test_pattern_match("[[]]", "[\\?]", true);

	test_pattern_match("[this is tml]", "[this is \\?]", true);
	test_pattern_match("[this is tml]", "[this \\? tml]", true);
	test_pattern_match("[this is tml]", "[\\? is tml]", true);
	test_pattern_match("[this is tml]", "[\\? is \\?]", true);

	test_pattern_match("[test is tml]", "[this is \\?]", false);
	test_pattern_match("[test is tml]", "[this \\? tml]", false);

	test_pattern_match("[[a b] [c d] [e f]]", "[[\\? b] [c \\?] [\\? f]]", true);
	test_pattern_match("[[a b] [c d] [e f]]", "[ \\? \\? | \\? \\? | \\? \\? ]", true);
	test_pattern_match("[[a b] [c d] [e f]]", "[[\\*] [\\? \\*] [\\? \\? \\*]]", true);

	test_pattern_match("[bold | hello, this is a test!]", "[bold|\\*]", true);
	test_pattern_match("[bold | hello, this is a test!]", "[italic|\\*]", false);
	test_pattern_match("[bold | hello, [italic | this] is a test!]", "[bold|\\*]", true);


	print_report();

	return 0;
}
