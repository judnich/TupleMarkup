CC = gcc -std=c89 -Wall -g
CCP = g++ -Wall -g

all: test_tml

run: all
	./test_tml

test_tml: test_tml.o tml_parser.o tml_tokenizer.o
	$(CCP) test_tml.o tml_parser.o tml_tokenizer.o -o test_tml

tml_tokenizer.o: ../../tml-c/source/tml_tokenizer.c ../../tml-c/source/tml_tokenizer.h
	$(CC) -c ../../tml-c/source/tml_tokenizer.c

tml_parser.o: ../../tml-c/source/tml_parser.c ../../tml-c/source/tml_parser.h
	$(CC) -c ../../tml-c/source/tml_parser.c

test_tml.o: test_tml.cpp ../source/tml.hpp
	$(CCP) -c test_tml.cpp

clean:
	rm -rf *.o test_tml
