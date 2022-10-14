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

    std::string look();
    std::string look(std::string obj);
    std::string go(std::string dir);
    std::string loc();
    std::vector<std::string> inv();
    std::vector<std::string> exits();
    std::vector<std::string> avail();
    std::string take(std::string obj);
    std::string drop(std::string obj);
    std::string use(std::string obj);

private:
    std::string tick();
    std::string input(std::string_view cmd);

    VM vm_;
    State state_;
    std::string prompt_;
};

#endif  // GAME_H_
