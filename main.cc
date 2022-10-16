#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
#include <variant>
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

using Memo = std::array<std::array<std::optional<uint16_t>, kMaxInt>, 5>;

uint16_t verify_reg8(uint16_t r0, uint16_t r1, uint16_t r7, Memo& memo) {
    /*
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

    assert(r0 < 5);
    assert(r1 < kMaxInt);
    if (r0 == 0) return (r1 + 1) % kMaxInt;
    if (auto val = memo[r0][r1]; val.has_value()) return *val;
    if (r1 > 0) {
        uint16_t y = verify_reg8(r0, (r1 + 32767) % kMaxInt, r7, memo);
        uint16_t x = verify_reg8((r0 + 32767) % kMaxInt, y, r7, memo);
        memo[r0][r1] = x;
        return x;
    } else {
        uint16_t x = verify_reg8((r0 + 32767) % kMaxInt, r7, r7, memo);
        memo[r0][r1] = x;
        return x;
    }
}

uint16_t compute_reg8() {
    for (uint16_t r7 = 0; r7 < kMaxInt; r7++) {
        Memo memo;
        auto result = verify_reg8(4, 1, r7, memo);
        if (result == 6) return r7;
    }
    assert(false);
}

using pt = std::pair<int, int>;
enum class Dir { North, South, East, West };

pt dir_to_d(Dir dir) {
    switch (dir) {
        case Dir::North: return {1, 0};
        case Dir::South: return {-1, 0};
        case Dir::East: return {0, 1};
        case Dir::West: return {0, -1};
    }
}

int dist(int row1, int col1, int row2, int col2) {
    int dr = row2 - row1;
    int dc = col2 - col1;
    return (dr > 0 ? dr : -dr) + (dc > 0 ? dc : -dc);
}

using tile = std::variant<int, char>;

static std::array<std::array<tile, 4>, 4> kGrid{
    std::array<tile, 4>{22, '-', 9, '*'},
    std::array<tile, 4>{'+', 4, '-', 18},
    std::array<tile, 4>{4, '*', 11, '*'},
    std::array<tile, 4>{'*', 8, '-', 1},
};

constexpr int kTarget = 30;
constexpr int kStart = 22;

static std::pair<Dir, std::array<Dir, 3>> kMoves[] = {
    {Dir::North, {Dir::North, Dir::East, Dir::West}},
    {Dir::South, {Dir::South, Dir::East, Dir::West}},
    {Dir::East, {Dir::East, Dir::North, Dir::South}},
    {Dir::West, {Dir::West, Dir::North, Dir::South}},
};

int apply(int base, char op, int val) {
    switch (op) {
        case '+': return base + val;
        case '*': return base * val;
        case '-': return base - val;
    }
    assert(false);
}

using Step = std::pair<Dir, int>;

bool path_exists(int row, int col, int sum, int steps,
                 std::vector<Step>& path) {
    if (row == 3 && col == 3) return sum == kTarget;
    if (steps <= 0) return false;
    for (const auto& [step1, step2s] : kMoves) {
        for (const auto step2 : step2s) {
            auto [dr1, dc1] = dir_to_d(step1);
            auto [dr2, dc2] = dir_to_d(step2);
            auto next_row = row + dr1 + dr2;
            auto next_col = col + dc1 + dc2;
            if (dist(next_row, next_col, 3, 3) > steps) continue;
            if (next_row < 0 || next_row > 3 || next_col < 0 || next_col > 3) {
                continue;
            }
            if (next_row == 0 && next_col == 0) continue;
            char op = std::get<char>(kGrid[row + dr1][col + dc1]);
            int val = std::get<int>(kGrid[next_row][next_col]);
            int next_sum = apply(sum, op, val);
            if (next_sum <= 0) continue;
            path.push_back({step1, sum});
            path.push_back({step2, next_sum});
            if (path_exists(next_row, next_col, next_sum, steps - 2, path)) {
                return true;
            }
            path.pop_back();
            path.pop_back();
        }
    }
    return false;
}

std::string solve_orb_path(Game& game) {
    printf("finding orb path...\n");
    std::vector<Step> path;
    bool exists = false;
    // i found a path manually in 16 steps that took too long, so we need
    // one shorter than that
    for (int steps = 6; steps <= 14; steps += 2) {
        if (path_exists(0, 0, kStart, steps, path)) {
            exists = true;
            break;
        }
    }
    assert(exists);
    printf("path in %lu\n", path.size());
    std::string last;
    for (auto [dir, sum] : path) {
        switch (dir) {
            case Dir::North: last = game.input("north"); break;
            case Dir::South: last = game.input("south"); break;
            case Dir::East: last = game.input("east"); break;
            case Dir::West: last = game.input("west"); break;
        }
    }
    return last;
}

void play(Game& game) {
    game.input("take tablet");
    game.input("use tablet");
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
    std::cout << "computing teleporter register..." << std::endl;
    auto val = compute_reg8();
    std::cout << "teleporter register: " << val << std::endl;
    game.set_8th_reg(val);
    game.input("use teleporter");
    game.input("north");
    game.input("north");
    game.input("north");
    game.input("north");
    game.input("north");
    game.input("north");
    game.input("north");
    game.input("east");
    game.input("take journal");
    game.input("west");
    game.input("north");
    game.input("north");
    game.input("take orb");
    solve_orb_path(game);
    game.input("vault");
    game.input("take mirror");
    std::cout << game.input("use mirror") << std::endl;
}

void run(vector<uint16_t> program) {
    Game game(program);
    play(game);
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
