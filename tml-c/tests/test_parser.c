#include "../source/tml_parser.h"

#include <stdio.h>

int main(void)
{
	struct tml_data *data = tml_parse_string("[1 2 3]");

	struct tml_node root = tml_data_root(data);

	


	tml_free_data(data);
}