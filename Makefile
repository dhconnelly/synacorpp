CC=clang++
CFLAGS=--std=c++17

synacorpp: main.o
	$(CC) $(LDFLAGS) main.o -o synacorpp

main.o: main.cc
	$(CC) $(CFLAGS) -c main.cc -o main.o

.PHONY: clean

clean:
	rm -rf *.o synacorpp
