#ifndef GAME_H_
#define GAME_H_

#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "vm.h"

class Game {
public:
    enum class State {
        WaitingForInput,
        GameOver,
    };

    Game(std::vector<uint16_t> program) : vm_(program) { tick(); }
    State state() const;

    std::string loc();
    std::string input(std::string_view cmd);

private:
    std::string tick();

    VM vm_;
    State state_;
    std::string prompt_;
};

#endif  // GAME_H_
