#include "game.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace {
static constexpr std::string_view kLocationDelimiter = "==";
}  // namespace

Game::State Game::state() const {
    switch (vm_.state()) {
        case VM::State::Halt: return State::GameOver;
        case VM::State::In: return State::WaitingForInput;
        case VM::State::Out:
        case VM::State::Run: assert(false);
    }
}

std::string Game::loc() {
    auto prompt = input("look");
    auto l = prompt.find(kLocationDelimiter);
    assert(l != std::string::npos);
    auto r = prompt.find(kLocationDelimiter, l + 1);
    assert(r != std::string::npos);
    return prompt.substr(l + 3, r - l - 4);
}

std::string Game::tick() {
    std::string buf;
    while (true) {
        vm_.step();
        switch (vm_.state()) {
            case VM::State::Halt: {
                state_ = State::GameOver;
                return buf;
            }
            case VM::State::In: {
                state_ = State::WaitingForInput;
                return buf;
            }
            case VM::State::Out: {
                buf.push_back(vm_.output());
            }
            case VM::State::Run: {
                // continue
            }
        }
    }
    return buf;
}

std::string Game::input(std::string_view cmd) {
    int i = 0;
    for (; i < cmd.size() && vm_.state() == VM::State::In; i++) {
        vm_.input(cmd[i]);
        tick();
    }
    if (i != cmd.size()) std::cerr << "warn: vm did not read entire command\n";
    vm_.input('\n');
    return tick();
}
