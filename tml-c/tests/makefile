CC = gcc -std=c89 -Wall -g

all: test_tokenizer test_parser

run: all
	./test_tokenizer; ./test_parser

test_tokenizer: test_tokenizer.o tml_tokenizer.o
	$(CC) test_tokenizer.o tml_tokenizer.o -o test_tokenizer

test_parser: test_parser.o tml_parser.o tml_tokenizer.o
	$(CC) test_parser.o tml_parser.o tml_tokenizer.o -o test_parser

test_tokenizer.o: test_tokenizer.c
	$(CC) -c test_tokenizer.c

test_parser.o: test_parser.c
	$(CC) -c test_parser.c

tml_tokenizer.o: ../source/tml_tokenizer.c ../source/tml_tokenizer.h
	$(CC) -c ../source/tml_tokenizer.c

tml_parser.o: ../source/tml_parser.c ../source/tml_parser.h
	$(CC) -c ../source/tml_parser.c

clean:
	rm -rf *.o test_tokenizer test_parser
