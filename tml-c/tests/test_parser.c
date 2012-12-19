#include "../source/tml_parser.h"

#include <stdio.h>

int main(void)
{
	struct tml_data *data = tml_parse_string("[ bold | this is a [italic | test], testing one two three! | hi ]");

	struct tml_node root = tml_data_root(data);

	char buff[1024];
	int s = tml_node_to_markup_string(&root, buff, 1024);

	printf("%s\n%d\n%d\n%s\n", tml_parse_error(data), data->buff_allocated, s, buff);

	tml_free_data(data);

	return 0;
}
