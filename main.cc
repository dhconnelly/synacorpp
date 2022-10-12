#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "vm.h"

using namespace std;

void die(string_view msg) {
    cerr << msg << endl;
    exit(1);
}

vector<uint16_t> read_program(istream& is) {
    vector<uint16_t> program;
    uint16_t lo, hi;
    while (is.good()) {
        lo = is.get();
        if (!is.good()) break;
        hi = is.get();
        if (!is.good()) break;
        program.push_back(lo + (hi << 8));
    }
    if (!is.eof()) die("error reading input file");
    return program;
}

void run(vector<uint16_t> program) {
    VM vm(program);
    while (vm.state() != VM::State::Halt) {
        if (vm.state() == VM::State::Out) cout << vm.output();
        if (vm.state() == VM::State::In) {
            char ch = cin.get();
            if (!cin.good()) break;
            vm.input(ch);
        }
        try {
            vm.step();
        } catch (const exception& e) {
            die(e.what());
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) die("usage: synacorpp <bin>");
    ifstream is(argv[1]);
    if (!is.good()) die(strerror(errno));
    auto program = read_program(is);
    run(program);
    return 0;
}
