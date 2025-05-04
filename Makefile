CC=gcc

libmarinara: marinara.o glad.o
	$(CC) -shared -o libmarinara.so *.o
glad.o: include/glad.c
	$(CC) -I./include -c -fpic include/glad.c -o glad.o
marinara.o: marinara.c marinara.h
	$(CC) -I./include -c -fpic marinara.c -o marinara.o
clean:
	rm libmarinara.so
	rm marinara.o
	rm glad.o
