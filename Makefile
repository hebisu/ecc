ecc: ecc.c

test: ecc
	./test.sh

clean:
	rm -f ecc *.o *~ tmp*