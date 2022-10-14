#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "game.h"

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
    Game game(program);
    std::string cmd;
    while (game.state() != Game::State::GameOver) {
        while (true) {
            printf("[%s] > ", game.loc().c_str());
            std::getline(cin, cmd);
            if (!cin.good()) return;
            if (cmd.find("look") == 0) {
                std::cout << game.look(cmd.substr(4)) << std::endl;
            } else if (cmd.find("go") == 0) {
                std::cout << game.go(cmd.substr(2)) << std::endl;
            } else if (cmd.find("exits") == 0) {
                for (const auto& item : game.exits()) {
                    std::cout << item << std::endl;
                }
            } else if (cmd.find("avail") == 0) {
                for (const auto& item : game.avail()) {
                    std::cout << item << std::endl;
                }
            } else if (cmd.find("inv") == 0) {
                for (const auto& item : game.inv()) {
                    std::cout << item << std::endl;
                }
            } else if (cmd.find("take") == 0) {
                std::cout << game.take(cmd.substr(4)) << std::endl;
            } else if (cmd.find("drop") == 0) {
                std::cout << game.drop(cmd.substr(4)) << std::endl;
            } else if (cmd.find("use") == 0) {
                std::cout << game.use(cmd.substr(3)) << std::endl;
            } else {
                fprintf(stderr, "invalid command: %s\n", cmd.c_str());
                continue;
            }
            break;
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
