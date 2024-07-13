all: clean example2

example2: example2.c mems.h
	gcc -o example2 example2.c

clean:
	rm -rf example2
