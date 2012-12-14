all: test_tokenizer

run: test_tokenizer
	./test_tokenizer

test_tokenizer: test_tokenizer.o tml_tokenizer.o
	gcc -Wall test_tokenizer.o tml_tokenizer.o -o test_tokenizer

tml_tokenizer.o: ../source/tml_tokenizer.c
	gcc -Wall -c ../source/tml_tokenizer.c

tml_parser.o: ../source/tml_parser.c
	gcc -Wall -c ../source/tml_parser.c

test_tokenizer.o: test_tokenizer.c
	gcc -Wall -c test_tokenizer.c

clean:
	rm -rf *.o test_tokenizer
