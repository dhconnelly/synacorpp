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
    game.take("tablet");
    game.go("doorway");
    game.go("north");
    game.go("north");
    game.go("bridge");
    game.go("continue");
    game.go("down");
    game.go("east");
    game.take("empty lantern");
    game.go("west");
    game.go("west");
    game.go("passage");
    game.go("ladder");
    game.go("west");
    game.go("north");
    game.go("north");
    game.go("south");
    game.go("north");
    game.take("can");
    game.go("west");
    game.go("ladder");
    game.go("darkness");
    game.use("can");
    game.use("lantern");
    game.go("continue");
    game.go("west");
    game.go("west");
    game.go("west");
    game.go("west");
}

using pt = std::pair<int, int>;

using ExploreNode = std::tuple<Game, pt, int>;

constexpr int kMaxExplore = 3000;

void explore(Game& game, std::map<pt, char>& m) {
    int tiles = 0;
    std::set<pt> v;
    v.insert({0, 0});
    m[{0, 0}] = 'X';
    std::deque<ExploreNode> q;
    // int max_dist = std::numeric_limits<int>::min();
    q.push_back({Game(game), {0, 0}, 0});
    while (!q.empty()) {
        auto [state, loc, dist] = q.front();
        q.pop_front();
        if (++tiles > kMaxExplore) {
            game = state;
            return;
        }
        // if (dist > max_dist) {
        // max_dist = dist;
        //  printf("dist = %d\n", dist);
        //}

        for (const auto& exit : state.exits()) {
            pt nbr;
            if (exit == "north") nbr = {loc.first, loc.second + 1};
            else if (exit == "south") nbr = {loc.first, loc.second - 1};
            else if (exit == "east") nbr = {loc.first + 1, loc.second};
            else if (exit == "west") nbr = {loc.first - 1, loc.second};
            else if (exit == "ladder") {
                if (m.find(loc) == m.end()) m[loc] = 'H';
                continue;
            } else assert(false);
            if (v.count(nbr) > 0) continue;
            v.insert(nbr);
            Game next(state);
            next.go(exit);
            std::cout << next.loc() << std::endl;
            if (next.loc() == "Panicked and lost") {
                m[nbr] = 'o';
                continue;
            }
            if (next.loc().find("Fumbling around") == 0) {
                m[nbr] = 'O';
                continue;
            }
            if (next.loc() != "Twisty passages" || next.avail().size() > 0) {
                game = next;
                return;
            }
            q.push_back({next, nbr, dist + 1});
        }
        if (m.find(loc) == m.end()) m[loc] = ' ';
    }
}

int min(int a, int b) { return a < b ? a : b; }

int max(int a, int b) { return a > b ? a : b; }

void print_map(std::map<pt, char>& m) {
    printf("printing: %lu\n", m.size());
    std::ofstream os("map.txt");
    int min_x = std::numeric_limits<int>::max();
    int min_y = std::numeric_limits<int>::max();
    int max_x = std::numeric_limits<int>::min();
    int max_y = std::numeric_limits<int>::min();
    for (const auto& [pt, ch] : m) {
        const auto& [x, y] = pt;
        min_x = min(min_x, x);
        min_y = min(min_y, y);
        max_x = max(max_x, x);
        max_y = max(max_y, y);
    }
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            auto ch = m[{x, y}];
            if (ch != 0) std::cout << ch;
            else std::cout << '#';
        }
        std::cout << std::endl;
    }
}

void run(vector<uint16_t> program) {
    Game game(program);
    navigate_to_maze(game);

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
                std::cout << game.input(cmd) << std::endl;
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
