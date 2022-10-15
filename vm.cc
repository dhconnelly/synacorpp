#include "vm.h"

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>

static constexpr uint16_t kMaxInt = (1 << 15);

static bool trace() { return getenv("TRACE") != nullptr; }

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
        case Opcode::Rmem: return Opcode::Rmem;
        case Opcode::Wmem: return Opcode::Wmem;
        case Opcode::Call: return Opcode::Call;
        case Opcode::Ret: return Opcode::Ret;
        case Opcode::Out: return Opcode::Out;
        case Opcode::In: return Opcode::In;
        case Opcode::Noop: return Opcode::Noop;
    }
    throw std::invalid_argument("bad opcode: " + std::to_string(op));
}

bool is_opcode(uint16_t val) {
    return val >= static_cast<uint16_t>(Opcode::Halt) &&
           val <= static_cast<uint16_t>(Opcode::Noop);
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
        case Opcode::Rmem: return 2;
        case Opcode::Wmem: return 2;
        case Opcode::Call: return 1;
        case Opcode::Ret: return 0;
        case Opcode::Out: return 1;
        case Opcode::In: return 1;
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
        case Opcode::Rmem: return "RMEM";
        case Opcode::Wmem: return "WMEM";
        case Opcode::Call: return "CALL";
        case Opcode::Ret: return "RET";
        case Opcode::Out: return "OUT";
        case Opcode::In: return "IN";
        case Opcode::Noop: return "NOOP";
    }
}

std::string value_string(uint16_t val) {
    if (val < kMaxInt) return std::to_string(val);
    if (auto reg = val - kMaxInt; reg < kNumReg)
        return "r" + std::to_string(reg);
    throw std::invalid_argument("invalid number: " + std::to_string(val));
}

VM::VM(std::vector<uint16_t> program) {
    for (size_t i = 0; i < program.size(); i++) memset(i, program[i]);
    for (size_t i = 0; i < reg_.size(); i++) reg_[i] = 0;
}

uint16_t VM::get(uint16_t val) const {
    if (val < kMaxInt) return val;
    if (auto reg = val - kMaxInt; reg < kNumReg) return reg_[reg];
    throw std::invalid_argument("invalid number: " + std::to_string(val));
}

void VM::set(uint16_t loc, uint16_t val) {
    if (loc < kMaxInt || loc >= kMaxInt + 8) {
        throw std::invalid_argument("invalid register: " + std::to_string(loc));
    }
    auto reg = loc - kMaxInt;
    reg_[reg] = val;
}

uint16_t VM::pop() {
    if (stack_.empty()) throw std::out_of_range("stack empty");
    auto val = stack_.back();
    stack_.pop_back();
    return val;
}

VM::Instr VM::load() {
    auto op = to_opcode(memget(pc_));
    return {op, memget(pc_ + 1), memget(pc_ + 2), memget(pc_ + 3)};
}

void disasm(const std::vector<uint16_t>& prog) {
    for (int pc = 0; pc < prog.size();) {
        auto x = prog[pc];
        if (!is_opcode(x)) {
            printf("[%8u] %u\n", pc, x);
            pc++;
            continue;
        }
        auto op = to_opcode(x);
        int n = arity(op);
        printf("[%8u] %s", pc, to_string(op));
        if (n > 0) printf(" %s", value_string(prog[pc + 1]).c_str());
        if (n > 1) printf(" %s", value_string(prog[pc + 2]).c_str());
        if (n > 2) printf(" %s", value_string(prog[pc + 3]).c_str());
        printf("\n");
        pc += n + 1;
    }
}

void VM::exec(Instr instr) {
    const auto& [op, a, b, c] = instr;
    if (trace()) {
        int n = arity(op);
        fprintf(stderr, "[%8u] %s", pc_, to_string(op));
        if (n > 0) fprintf(stderr, " %s", value_string(a).c_str());
        if (n > 1) fprintf(stderr, " %s", value_string(b).c_str());
        if (n > 2) fprintf(stderr, " %s", value_string(c).c_str());
        fprintf(stderr, "\n");
    }
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
            set(a, pop());
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

        case Opcode::Rmem: {
            set(a, memget(get(b)));
            break;
        }

        case Opcode::Wmem: {
            memset(get(a), get(b));
            break;
        }

        case Opcode::Call: {
            if (pc_ == 5489) {
                reg_[0] = 6;
                printf("skipping verification\n");
                break;
            }
            stack_.push_back(next_pc);
            next_pc = get(a);
            break;
        }

        case Opcode::Ret: {
            if (stack_.empty()) {
                state_ = State::Halt;
                break;
            }
            next_pc = pop();
            break;
        }

        case Opcode::Out: {
            state_ = State::Out;
            out_ = get(a);
            break;
        }

        case Opcode::In: {
            state_ = State::In;
            next_pc = pc_;
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

    // handle previous input
    if (state_ == State::In) {
        const auto& [op, a, b, c] = load();
        set(a, in_);
        pc_ = pc_ + arity(Opcode::In) + 1;
    }

    state_ = State::Run;
    exec(load());
}
