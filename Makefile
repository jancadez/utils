HEADERS = $(wildcard ./*.h)

all: main

main: main.c $(HEADERS)
	gcc main.c -o main -Wall -Wextra

run: all
	./main

.PHONY: all run
