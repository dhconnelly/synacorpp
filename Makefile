CC=clang++
CFLAGS=--std=c++17

synacorpp: main.o vm.o
	$(CC) $(LDFLAGS) $^ -o synacorpp

main.o: main.cc vm.h
	$(CC) $(CFLAGS) -c main.cc -o main.o

vm.o: vm.h
	$(CC) $(CFLAGS) -c vm.cc -o vm.o

.PHONY: clean

clean:
	rm -rf *.o synacorpp
