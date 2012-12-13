all: test_tokenizer

run: test_tokenizer
	./test_tokenizer

test_tokenizer: test_tokenizer.o tml_tokenizer.o
	gcc test_tokenizer.o tml_tokenizer.o -o test_tokenizer

tml_tokenizer.o: ../source/tml_tokenizer.c ../source/tml_tokenizer.h
	gcc -c ../source/tml_tokenizer.c

test_tokenizer.o: test_tokenizer.c
	gcc -c test_tokenizer.c

clean:
	rm -rf *.o test_tokenizer
