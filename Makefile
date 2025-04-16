CC=gcc

libmarinara: marinara.o
	$(CC) -shared -o libmarinara.so marinara.o
marinara.o:
	$(CC) -c -fpic marinara.c -o marinara.o
clean:
	rm marinara.o
