#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
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

void navigate_to_maze(Game& game) {
    game.input("take tablet");
    game.input("doorway");
    game.input("north");
    game.input("north");
    game.input("bridge");
    game.input("continue");
    game.input("down");
    game.input("east");
    game.input("take empty lantern");
    game.input("west");
    game.input("west");
    game.input("passage");
    game.input("ladder");
    game.input("west");
    game.input("north");
    game.input("north");
    game.input("south");
    game.input("north");
    game.input("take can");
    game.input("west");
    game.input("ladder");
    game.input("darkness");
    game.input("use can");
    game.input("use lantern");
    game.input("continue");
    game.input("west");
    game.input("west");
    game.input("west");
    game.input("west");
}

void run(vector<uint16_t> program) {
    Game game(program);
    navigate_to_maze(game);
    std::string cmd;
    while (game.state() != Game::State::GameOver) {
        printf("[%s] > ", game.loc().c_str());
        std::getline(cin, cmd);
        if (!cin.good()) return;
        std::cout << game.input(cmd) << std::endl;
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
