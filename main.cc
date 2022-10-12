#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

using namespace std;

void die(string_view msg) { cerr << msg << endl; }

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

static constexpr uint16_t kMaxInt = (1 << 15) - 1;

enum class Opcode : uint16_t { Halt = 0, Out = 19, Noop = 21 };
using Instr = tuple<Opcode, uint16_t, uint16_t, uint16_t>;

static constexpr int arity(Opcode op) {
    switch (op) {
        case Opcode::Halt: return 0;
        case Opcode::Out: return 1;
        case Opcode::Noop: return 0;
    }
}

const char* to_string(Opcode op) {
    switch (op) {
        case Opcode::Halt: return "HALT";
        case Opcode::Out: return "OUT";
        case Opcode::Noop: return "NOOP";
    }
}

class VM final {
public:
    enum class State {
        Run,
        Halt,
        Out,
    };

    VM(vector<uint16_t> program);
    State state() const { return state_; }
    void step();
    char output() const { return out_; };
    bool done() const { return state_ == State::Halt; }
    void enable_trace(bool enable) { trace_ = enable; }

private:
    Instr load();
    void exec(Instr instr);
    uint16_t get(uint16_t val) const;

    static constexpr size_t kNumReg = 8;

    bool trace_ = false;
    uint16_t pc_ = 0;
    char out_ = '\0';
    map<uint16_t, uint16_t> mem_;
    vector<uint16_t> stack_;
    array<uint16_t, kNumReg> reg_;
    State state_ = State::Run;
};

VM::VM(vector<uint16_t> program) {
    for (size_t i = 0; i < program.size(); i++) mem_[i] = program[i];
}

uint16_t VM::get(uint16_t val) const {
    if (val <= kMaxInt) return val;
    if (auto reg = val - kMaxInt - 1; reg < kNumReg) return reg_[reg];
    throw new out_of_range("invalid number: " + to_string(val));
}

Instr VM::load() {
    auto op = static_cast<Opcode>(mem_[pc_]);
    return {op, mem_[pc_ + 1], mem_[pc_ + 2], mem_[pc_ + 3]};
}

void VM::exec(Instr instr) {
    const auto& [op, a, b, c] = instr;
    if (trace_) printf("[%4u] %s\n", pc_, to_string(op));
    auto next_pc = pc_ + arity(op) + 1;
    switch (op) {
        case Opcode::Halt: {
            state_ = State::Halt;
            break;
        }

        case Opcode::Out: {
            state_ = State::Out;
            out_ = get(a);
            break;
        }

        case Opcode::Noop: {
            break;
        }
    }
    pc_ = next_pc;
}

void VM::step() {
    if (state_ == State::Halt) return;
    exec(load());
}

int main(int argc, char* argv[]) {
    if (argc != 2) die("usage: synacorpp <bin>");
    ifstream is(argv[1]);
    if (!is.good()) die(strerror(errno));
    auto prog = read_program(is);
    VM vm(prog);
    if (getenv("TRACE") != nullptr) vm.enable_trace(true);
    while (!vm.done()) {
        if (vm.state() == VM::State::Out) cout << vm.output();
        vm.step();
    }
    return 0;
}
