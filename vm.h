#ifndef VM_H_
#define VM_H_

#include <array>
#include <cstdint>
#include <map>
#include <vector>

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
    Rmem = 15,
    Wmem = 16,
    Call = 17,
    Ret = 18,
    Out = 19,
    In = 20,
    Noop = 21
};

class VM final {
public:
    enum class State {
        Run,
        Halt,
        Out,
        In,
    };

    VM(std::vector<uint16_t> program);
    State state() const { return state_; }
    void step();
    char output() const { return out_; }
    void input(char ch) { in_ = ch; }

private:
    using Instr = std::tuple<Opcode, uint16_t, uint16_t, uint16_t>;

    Instr load();
    void exec(Instr instr);
    uint16_t get(uint16_t val) const;
    void set(uint16_t loc, uint16_t val);
    uint16_t pop();

    static constexpr size_t kNumReg = 8;

    uint16_t pc_ = 0;
    char out_ = 0;
    char in_ = 0;
    std::map<uint16_t, uint16_t> mem_;
    std::vector<uint16_t> stack_;
    std::array<uint16_t, kNumReg> reg_;
    State state_ = State::Run;
};

#endif  // VM_H_
