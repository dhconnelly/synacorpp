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

static constexpr uint16_t kMaxInt = (1 << 15);

enum class Opcode : uint16_t {
    Halt = 0,
    Set = 1,
    Push = 2,
    Pop = 3,
    Eq = 4,
    Gt = 5,
    Jmp = 6,
    Jt = 7,
    Jf = 8,
    Add = 9,
    Mult = 10,
    Mod = 11,
    And = 12,
    Or = 13,
    Not = 14,
    Out = 19,
    Noop = 21
};

static constexpr Opcode to_opcode(uint16_t op) {
    switch (static_cast<Opcode>(op)) {
        case Opcode::Halt: return Opcode::Halt;
        case Opcode::Set: return Opcode::Set;
        case Opcode::Push: return Opcode::Push;
        case Opcode::Pop: return Opcode::Pop;
        case Opcode::Eq: return Opcode::Eq;
        case Opcode::Gt: return Opcode::Gt;
        case Opcode::Jmp: return Opcode::Jmp;
        case Opcode::Jt: return Opcode::Jt;
        case Opcode::Jf: return Opcode::Jf;
        case Opcode::Add: return Opcode::Add;
        case Opcode::Mult: return Opcode::Mult;
        case Opcode::Mod: return Opcode::Mod;
        case Opcode::And: return Opcode::And;
        case Opcode::Or: return Opcode::Or;
        case Opcode::Not: return Opcode::Not;
        case Opcode::Out: return Opcode::Out;
        case Opcode::Noop: return Opcode::Noop;
    }
    throw invalid_argument("bad opcode: " + to_string(op));
}

static constexpr int arity(Opcode op) {
    switch (op) {
        case Opcode::Halt: return 0;
        case Opcode::Set: return 2;
        case Opcode::Push: return 1;
        case Opcode::Pop: return 1;
        case Opcode::Eq: return 3;
        case Opcode::Gt: return 3;
        case Opcode::Jmp: return 1;
        case Opcode::Jt: return 2;
        case Opcode::Jf: return 2;
        case Opcode::Add: return 3;
        case Opcode::Mult: return 3;
        case Opcode::Mod: return 3;
        case Opcode::And: return 3;
        case Opcode::Or: return 3;
        case Opcode::Not: return 2;
        case Opcode::Out: return 1;
        case Opcode::Noop: return 0;
    }
}

const char* to_string(Opcode op) {
    switch (op) {
        case Opcode::Halt: return "HALT";
        case Opcode::Set: return "SET";
        case Opcode::Push: return "PUSH";
        case Opcode::Pop: return "POP";
        case Opcode::Eq: return "EQ";
        case Opcode::Gt: return "GT";
        case Opcode::Jmp: return "JMP";
        case Opcode::Jt: return "JT";
        case Opcode::Jf: return "JF";
        case Opcode::Add: return "ADD";
        case Opcode::Mult: return "MULT";
        case Opcode::Mod: return "MOD";
        case Opcode::And: return "AND";
        case Opcode::Or: return "OR";
        case Opcode::Not: return "NOT";
        case Opcode::Out: return "OUT";
        case Opcode::Noop: return "NOOP";
    }
}

using Instr = tuple<Opcode, uint16_t, uint16_t, uint16_t>;

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
    void set(uint16_t loc, uint16_t val);

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
    for (size_t i = 0; i < reg_.size(); i++) reg_[i] = 0;
}

uint16_t VM::get(uint16_t val) const {
    if (val < kMaxInt) return val;
    if (auto reg = val - kMaxInt; reg < kNumReg) return reg_[reg];
    throw invalid_argument("invalid number: " + to_string(val));
}

void VM::set(uint16_t loc, uint16_t val) {
    if (loc < kMaxInt || loc >= kMaxInt + 8) {
        throw invalid_argument("invalid register: " + to_string(loc));
    }
    auto reg = loc - kMaxInt;
    reg_[reg] = val;
}

Instr VM::load() {
    auto op = to_opcode(mem_[pc_]);
    return {op, mem_[pc_ + 1], mem_[pc_ + 2], mem_[pc_ + 3]};
}

void VM::exec(Instr instr) {
    const auto& [op, a, b, c] = instr;
    if (trace_) printf("[%8u] %s %u %u %u\n", pc_, to_string(op), a, b, c);
    auto next_pc = pc_ + arity(op) + 1;
    switch (op) {
        case Opcode::Halt: {
            state_ = State::Halt;
            break;
        }

        case Opcode::Set: {
            set(a, get(b));
            break;
        }

        case Opcode::Push: {
            stack_.push_back(get(a));
            break;
        }

        case Opcode::Pop: {
            if (stack_.empty()) throw out_of_range("stack empty");
            auto val = stack_.back();
            stack_.pop_back();
            set(a, val);
            break;
        }

        case Opcode::Eq: {
            set(a, get(b) == get(c));
            break;
        }

        case Opcode::Gt: {
            set(a, get(b) > get(c));
            break;
        }

        case Opcode::Jmp: {
            next_pc = get(a);
            break;
        }

        case Opcode::Jt: {
            auto cond = get(a);
            if (cond != 0) next_pc = get(b);
            break;
        }

        case Opcode::Jf: {
            auto cond = get(a);
            if (cond == 0) next_pc = get(b);
            break;
        }

        case Opcode::Add: {
            auto result = (get(b) + get(c)) % kMaxInt;
            set(a, result);
            break;
        }

        case Opcode::Mult: {
            auto result = (get(b) * get(c)) % kMaxInt;
            set(a, result);
            break;
        }

        case Opcode::Mod: {
            auto result = (get(b) % get(c)) % kMaxInt;
            set(a, result);
            break;
        }

        case Opcode::And: {
            auto result = (get(b) & get(c));
            set(a, result);
            break;
        }

        case Opcode::Or: {
            auto result = (get(b) | get(c));
            set(a, result);
            break;
        }

        case Opcode::Not: {
            auto result = (~get(b)) & (kMaxInt - 1);
            set(a, result);
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

void run(vector<uint16_t> program) {
    VM vm(program);
    if (getenv("TRACE") != nullptr) vm.enable_trace(true);
    while (!vm.done()) {
        if (vm.state() == VM::State::Out) cout << vm.output();
        try {
            vm.step();
        } catch (const exception& e) {
            die(e.what());
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
