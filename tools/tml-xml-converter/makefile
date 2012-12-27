CC = gcc -std=c99 -Wall -O3 

all: tml-convert

tml-convert: tml-convert.o tml_parser.o tml_tokenizer.o
	$(CC) `xml2-config --cflags --libs` tml-convert.o tml_parser.o tml_tokenizer.o -o tml-convert

tml-convert.o: tml-convert.c
	$(CC) `xml2-config --cflags` -c tml-convert.c

tml_tokenizer.o: ../../tml-c/source/tml_tokenizer.c ../../tml-c/source/tml_tokenizer.h
	$(CC) -c ../../tml-c/source/tml_tokenizer.c

tml_parser.o: ../../tml-c/source/tml_parser.c ../../tml-c/source/tml_parser.h
	$(CC) -c ../../tml-c/source/tml_parser.c

clean:
	rm -rf *.o tml-convert
	