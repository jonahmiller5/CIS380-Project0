default: shredder

shredder: shredder.c
	clang -Wall -o penn-shredder shredder.c
	
clean:
	rm -rf *.o

clobber: clean
	rm -rf penn-shredder