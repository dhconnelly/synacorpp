CC=clang++
CFLAGS=--std=c++17 -Wall -Werror

synacorpp: main.o vm.o game.o
	$(CC) $(LDFLAGS) $^ -o synacorpp

main.o: main.cc game.h
	$(CC) $(CFLAGS) -c main.cc -o main.o

vm.o: vm.h vm.cc
	$(CC) $(CFLAGS) -c vm.cc -o vm.o

game.o: game.cc vm.h
	$(CC) $(CFLAGS) -c game.cc -o game.o

.PHONY: clean

clean:
	rm -rf *.o synacorpp
