#ifndef GAME_H_
#define GAME_H_

#include <string>
#include <utility>
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

    Game(std::vector<uint16_t> program) : vm_(program) {}
    State state() const;
    void start();

    pt position() const;
    std::string_view location() const;
    const std::vector<std::string>& available() const;
    const std::vector<std::string>& directions() const;
    const std::vector<std::string>& inventory() const;
    std::string_view prompt() const;

    void input(std::string_view command);

private:
    VM vm_;
    State state_ = State::Off;
    pt position_ = {0, 0};
    std::string location_;
    std::vector<std::string> available_;
    std::vector<std::string> directions_;
    std::vector<std::string> inventory_;
    std::string prompt_;
};

#endif  // GAME_H_
