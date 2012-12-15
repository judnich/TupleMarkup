all: test_tokenizer test_parser

test_tokenizer: test_tokenizer.o tml_tokenizer.o
	gcc -Wall test_tokenizer.o tml_tokenizer.o -o test_tokenizer

test_parser: test_parser.o tml_parser.o tml_tokenizer.o
	gcc -Wall test_parser.o tml_parser.o tml_tokenizer.o -o test_parser

tml_tokenizer.o: ../source/tml_tokenizer.c ../source/tml_tokenizer.h
	gcc -Wall -c ../source/tml_tokenizer.c

tml_parser.o: ../source/tml_parser.c ../source/tml_parser.h
	gcc -Wall -c ../source/tml_parser.c

test_tokenizer.o: test_tokenizer.c
	gcc -Wall -c test_tokenizer.c

clean:
	rm -rf *.o test_tokenizer
