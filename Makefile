CC=gcc

libmarinara: marinara.o glad.o
	$(CC) -shared -o libmarinara.so *.o
glad.o:
	$(CC) -c -fpic include/glad.c -o glad.o
marinara.o:
	$(CC) -c -fpic marinara.c -o marinara.o
clean:
	rm marinara.o
	rm glad.o
