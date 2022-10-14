#include "game.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace {
static constexpr std::string_view kAvailablePrompt = " of interest here:\n";
static constexpr std::string_view kInventoryPrompt = " inventory:\n";
static constexpr std::string_view kExitsPrompt = " exits:\n";
static constexpr std::string_view kLocationDelimiter = "==";

std::vector<std::string> parse_list(std::string_view text,
                                    std::string_view prompt) {
    auto l = text.find(prompt);
    if (l == std::string::npos) return {};
    l += prompt.size();
    std::vector<std::string> list;
    while (l < text.size() && text[l] != '\n') {
        auto r = l;
        while (text[r] != '\n') r++;
        list.emplace_back(text.substr(l + 2, r - l - 2));
        l = r + 1;
    }
    return list;
}
}  // namespace

Game::State Game::state() const {
    switch (vm_.state()) {
        case VM::State::Halt: return State::GameOver;
        case VM::State::In: return State::WaitingForInput;
        case VM::State::Out:
        case VM::State::Run: assert(false);
    }
}

std::string Game::look() { return input("look"); }

std::string Game::look(std::string obj) { return input("look " + obj); }

std::string Game::go(std::string dir) { return input("go " + dir); }

std::string Game::loc() {
    auto prompt = look();
    auto l = prompt.find(kLocationDelimiter);
    assert(l != std::string::npos);
    auto r = prompt.find(kLocationDelimiter, l + 1);
    assert(r != std::string::npos);
    return prompt.substr(l + 3, r - l - 4);
}

std::vector<std::string> Game::inv() {
    return parse_list(input("inv"), kInventoryPrompt);
}

std::vector<std::string> Game::exits() {
    return parse_list(look(), kExitsPrompt);
}

std::vector<std::string> Game::avail() {
    return parse_list(look(), kAvailablePrompt);
}

std::string Game::take(std::string obj) { return input("take " + obj); }

std::string Game::drop(std::string obj) { return input("drop " + obj); }

std::string Game::use(std::string obj) { return input("use " + obj); }

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
