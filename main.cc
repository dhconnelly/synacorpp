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

uint16_t compute_reg8() {
    /*
    // verification algorithm
    r0 = 4
    r1 = 1
    r7 = input
    want result = 6

    [    6027] JT r0 6035
    [    6030] ADD r0 r1 1
    [    6034] RET
    [    6035] JT r1 6048
    [    6038] ADD r0 r0 32767
    [    6042] SET r1 r7
    [    6045] CALL 6027
    [    6047] RET
    [    6048] PUSH r0
    [    6050] ADD r1 r1 32767
    [    6054] CALL 6027
    [    6056] SET r1 r0
    [    6059] POP r0
    [    6061] ADD r0 r0 32767
    [    6065] CALL 6027
    [    6067] RET
    */
    return 1;
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
    game.input("north");
    game.input("take red coin");
    game.input("north");
    game.input("west");
    game.input("take blue coin");
    game.input("up");
    game.input("take shiny coin");
    game.input("down");
    game.input("east");
    game.input("east");
    game.input("take concave coin");
    game.input("down");
    game.input("take corroded coin");
    game.input("up");
    game.input("west");
    game.input("use blue coin");
    game.input("use red coin");
    game.input("use shiny coin");
    game.input("use concave coin");
    game.input("use corroded coin");
    game.input("north");
    game.input("take teleporter");
    game.input("use teleporter");
    game.input("take business card");
    game.input("take strange book");
    game.set_8th_reg(compute_reg8());
    std::cout << game.input("use teleporter") << std::endl;
}

void run(vector<uint16_t> program) {
    Game game(program);
    navigate_to_maze(game);
    std::string cmd;
    while (game.state() != Game::State::GameOver) {
        printf("[%s] > ", game.loc().c_str());
        std::getline(cin, cmd);
        if (!cin.good()) return;
        if (cmd.find("reg8") == 0) {
            auto s = cmd.substr(5);
            int val = atoi(s.data());
            game.set_8th_reg(val);
        } else {
            std::cout << game.input(cmd) << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) die("usage: synacorpp <cmd> <bin>\ncommands: run, disasm\n");
    ifstream is(argv[2]);
    if (!is.good()) die(strerror(errno));
    auto program = read_program(is);
    if (argv[1] == string("run")) run(program);
    if (argv[1] == string("disasm")) disasm(program);
    return 0;
}
