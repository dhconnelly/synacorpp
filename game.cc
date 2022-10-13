#include "game.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

void Game::tick() {
    while (true) {
        vm_.step();
        switch (vm_.state()) {
            case VM::State::Halt: {
                state_ = State::GameOver;
                return;
            }
            case VM::State::In: {
                state_ = State::WaitingForInput;
                return;
            }
            case VM::State::Out: {
                prompt_.push_back(vm_.output());
            }
            case VM::State::Run: {
                // continue
            }
        }
    }
}

void Game::input(std::string_view cmd) {
    std::cout << "reading input from [" << cmd << "]\n";
    int i = 0;
    for (; i < cmd.size() && vm_.state() == VM::State::In; i++) {
        std::cout << "writing: [" << cmd[i] << "]\n";
        vm_.input(cmd[i]);
        tick();
    }
    if (i != cmd.size()) {
        std::cerr << "warning: vm did not read entire command\n";
    }
    vm_.input('\n');
    prompt_.clear();
    tick();
}
