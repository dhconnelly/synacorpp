#ifndef GAME_H_
#define GAME_H_

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "vm.h"

using pt = std::pair<int, int>;

class Game {
public:
    enum class State {
        Off,
        WaitingForInput,
        GameOver,
    };

    Game(std::vector<uint16_t> program) : vm_(program) { tick(); }
    State state() const { return state_; }
    std::string_view prompt() const { return prompt_; }

    void look(std::string obj) { input("look" + obj); }
    void go(std::string dir) { input("go" + dir); }
    void inv() { input("inv"); }
    void take(std::string obj) { input("take" + obj); }
    void drop(std::string obj) { input("drop" + obj); }
    void use(std::string obj) { input("use" + obj); }

private:
    void tick();
    void input(std::string_view cmd);

    VM vm_;
    State state_ = State::Off;
    std::string prompt_;
};

#endif  // GAME_H_
