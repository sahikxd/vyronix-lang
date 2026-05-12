#include "vyronix/IROptimizer.hpp"
#include <stack>
#include <iostream>

namespace vyronix {

std::vector<Instruction> IROptimizer::optimize(const std::vector<Instruction>& code) {
    std::vector<Instruction> optimized = constantFolding(code);
    optimized = peephole(optimized);
    optimized = deadCodeElimination(optimized);
    return optimized;
}

std::vector<Instruction> IROptimizer::peephole(const std::vector<Instruction>& code) {
    std::vector<Instruction> result = code;
    bool changed = true;

    while (changed) {
        changed = false;
        for (size_t i = 0; i < result.size(); ++i) {
            if (result[i].opcode == OpCode::NOP) continue;

            // 1. PUSH_CONST x; POP -> NOP; NOP
            if (i + 1 < result.size() && 
                result[i].opcode == OpCode::PUSH_CONST && 
                result[i+1].opcode == OpCode::POP) {
                result[i] = Instruction(OpCode::NOP);
                result[i+1] = Instruction(OpCode::NOP);
                changed = true;
                continue;
            }

            // 2. STORE_VAR x; POP; LOAD_VAR x -> STORE_VAR x; NOP; NOP
            if (i + 2 < result.size() &&
                result[i].opcode == OpCode::STORE_VAR &&
                result[i+1].opcode == OpCode::POP &&
                result[i+2].opcode == OpCode::LOAD_VAR) {
                
                auto* s1 = std::get_if<std::string>(&result[i].operand);
                auto* s2 = std::get_if<std::string>(&result[i+2].operand);
                if (s1 && s2 && *s1 == *s2) {
                    result[i+1] = Instruction(OpCode::NOP);
                    result[i+2] = Instruction(OpCode::NOP);
                    changed = true;
                    continue;
                }
            }

            // 3. PUSH_CONST 0; ADD -> NOP; NOP
            if (i + 1 < result.size() && result[i].opcode == OpCode::PUSH_CONST && result[i+1].opcode == OpCode::ADD) {
                auto* i64 = std::get_if<int64_t>(&result[i].operand);
                auto* f64 = std::get_if<double>(&result[i].operand);
                if ((i64 && *i64 == 0) || (f64 && *f64 == 0.0)) {
                    result[i] = Instruction(OpCode::NOP);
                    result[i+1] = Instruction(OpCode::NOP);
                    changed = true;
                    continue;
                }
            }

            // 4. PUSH_CONST 1; MUL -> NOP; NOP
            if (i + 1 < result.size() && result[i].opcode == OpCode::PUSH_CONST && result[i+1].opcode == OpCode::MUL) {
                auto* i64 = std::get_if<int64_t>(&result[i].operand);
                auto* f64 = std::get_if<double>(&result[i].operand);
                if ((i64 && *i64 == 1) || (f64 && *f64 == 1.0)) {
                    result[i] = Instruction(OpCode::NOP);
                    result[i+1] = Instruction(OpCode::NOP);
                    changed = true;
                    continue;
                }
            }

            // 5. NOT; NOT -> NOP; NOP
            if (i + 1 < result.size() &&
                result[i].opcode == OpCode::NOT &&
                result[i+1].opcode == OpCode::NOT) {
                result[i] = Instruction(OpCode::NOP);
                result[i+1] = Instruction(OpCode::NOP);
                changed = true;
                continue;
            }

            // 6. JUMP target; ... target: next_instr -> NOP if target is i+1
            if (result[i].opcode == OpCode::JUMP) {
                if (auto* lbl = std::get_if<Label>(&result[i].operand)) {
                    int64_t target = lbl->index;
                    size_t next = i + 1;
                    while (next < result.size() && result[next].opcode == OpCode::NOP) next++;
                    if (target == (int64_t)next) {
                        result[i] = Instruction(OpCode::NOP);
                        changed = true;
                        continue;
                    }
                }
            }

            // 7. JUMP to another JUMP
            if (result[i].opcode == OpCode::JUMP || result[i].opcode == OpCode::JUMP_IF_FALSE) {
                if (auto* lbl = std::get_if<Label>(&result[i].operand)) {
                    int64_t target = lbl->index;
                    if (target >= 0 && (size_t)target < result.size() && result[target].opcode == OpCode::JUMP) {
                        if (auto* next_lbl = std::get_if<Label>(&result[target].operand)) {
                            if (next_lbl->index != target) {
                                result[i].operand = *next_lbl;
                                changed = true;
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

std::vector<Instruction> IROptimizer::constantFolding(const std::vector<Instruction>& code) {
    std::vector<Instruction> result = code;
    bool changed = true;

    while (changed) {
        changed = false;
        for (size_t i = 0; i < result.size(); ++i) {
            if (result[i].opcode == OpCode::NOP) continue;

            // Find last two non-NOP instructions
            int idx1 = -1, idx2 = -1;
            for (int j = (int)i - 1; j >= 0; --j) {
                if (result[j].opcode != OpCode::NOP) {
                    if (idx2 == -1) idx2 = j;
                    else { idx1 = j; break; }
                }
            }

            if (idx1 != -1 && idx2 != -1 && 
                result[idx1].opcode == OpCode::PUSH_CONST && 
                result[idx2].opcode == OpCode::PUSH_CONST) {
                
                auto& a = result[idx1].operand;
                auto& b = result[idx2].operand;
                bool folded = false;
                IRValue folded_val;

                if (result[i].opcode == OpCode::ADD) {
                    auto* i1 = std::get_if<int64_t>(&a); auto* i2 = std::get_if<int64_t>(&b);
                    auto* f1 = std::get_if<double>(&a); auto* f2 = std::get_if<double>(&b);
                    auto* s1 = std::get_if<std::string>(&a); auto* s2 = std::get_if<std::string>(&b);
                    
                    if (i1 && i2) { folded_val = *i1 + *i2; folded = true; }
                    else if (f1 && f2) { folded_val = *f1 + *f2; folded = true; }
                    else if (s1 && s2) { folded_val = *s1 + *s2; folded = true; }
                } else if (result[i].opcode == OpCode::SUB) {
                    auto* i1 = std::get_if<int64_t>(&a); auto* i2 = std::get_if<int64_t>(&b);
                    auto* f1 = std::get_if<double>(&a); auto* f2 = std::get_if<double>(&b);
                    if (i1 && i2) { folded_val = *i1 - *i2; folded = true; }
                    else if (f1 && f2) { folded_val = *f1 - *f2; folded = true; }
                } else if (result[i].opcode == OpCode::MUL) {
                    auto* i1 = std::get_if<int64_t>(&a); auto* i2 = std::get_if<int64_t>(&b);
                    auto* f1 = std::get_if<double>(&a); auto* f2 = std::get_if<double>(&b);
                    if (i1 && i2) { folded_val = *i1 * *i2; folded = true; }
                    else if (f1 && f2) { folded_val = *f1 * *f2; folded = true; }
                } else if (result[i].opcode == OpCode::DIV) {
                    auto* i1 = std::get_if<int64_t>(&a); auto* i2 = std::get_if<int64_t>(&b);
                    auto* f1 = std::get_if<double>(&a); auto* f2 = std::get_if<double>(&b);
                    if (i1 && i2 && *i2 != 0) { folded_val = *i1 / *i2; folded = true; }
                    else if (f1 && f2 && *f2 != 0.0) { folded_val = *f1 / *f2; folded = true; }
                } else if (result[i].opcode == OpCode::EQ) {
                    folded_val = (a == b); folded = true;
                } else if (result[i].opcode == OpCode::NE) {
                    folded_val = (a != b); folded = true;
                } else if (result[i].opcode == OpCode::LT) {
                    folded_val = (a < b); folded = true;
                } else if (result[i].opcode == OpCode::LE) {
                    folded_val = (a <= b); folded = true;
                } else if (result[i].opcode == OpCode::GT) {
                    folded_val = (a > b); folded = true;
                } else if (result[i].opcode == OpCode::GE) {
                    folded_val = (a >= b); folded = true;
                } else if (result[i].opcode == OpCode::AND) {
                    auto* b1 = std::get_if<bool>(&a); auto* b2 = std::get_if<bool>(&b);
                    if (b1 && b2) { folded_val = *b1 && *b2; folded = true; }
                } else if (result[i].opcode == OpCode::OR) {
                    auto* b1 = std::get_if<bool>(&a); auto* b2 = std::get_if<bool>(&b);
                    if (b1 && b2) { folded_val = *b1 || *b2; folded = true; }
                }

                if (folded) {
                    result[idx1] = Instruction(OpCode::NOP);
                    result[idx2] = Instruction(OpCode::NOP);
                    result[i] = Instruction(OpCode::PUSH_CONST, folded_val);
                    changed = true;
                    continue;
                }
            }

            // Folding for JUMP_IF_FALSE with constant
            if (idx2 != -1 && result[idx2].opcode == OpCode::PUSH_CONST && result[i].opcode == OpCode::JUMP_IF_FALSE) {
                if (auto* bval = std::get_if<bool>(&result[idx2].operand)) {
                    bool val = *bval;
                    result[idx2] = Instruction(OpCode::NOP);
                    if (val) {
                        result[i] = Instruction(OpCode::NOP);
                    } else {
                        result[i] = Instruction(OpCode::JUMP, result[i].operand);
                    }
                    changed = true;
                    continue;
                }
            }

            // Unary ops
            if (idx2 != -1 && result[idx2].opcode == OpCode::PUSH_CONST && result[i].opcode == OpCode::NOT) {
                if (auto* bval = std::get_if<bool>(&result[idx2].operand)) {
                    result[idx2] = Instruction(OpCode::NOP);
                    result[i] = Instruction(OpCode::PUSH_CONST, !(*bval));
                    changed = true;
                    continue;
                }
            }
        }
    }
    return result;
}

std::vector<Instruction> IROptimizer::deadCodeElimination(const std::vector<Instruction>& code) {
    std::cout << "[DEBUG] DCE start\n";
    if (code.empty()) return code;

    std::vector<bool> reachable(code.size(), false);
    std::stack<size_t> worklist;

    worklist.push(0);
    reachable[0] = true;

    while (!worklist.empty()) {
        size_t i = worklist.top();
        worklist.pop();

        const auto& instr = code[i];
        std::vector<size_t> next;
        
        if (instr.opcode == OpCode::JUMP || instr.opcode == OpCode::JUMP_IF_FALSE) {
            if (std::holds_alternative<Label>(instr.operand)) {
                next.push_back((size_t)std::get<Label>(instr.operand).index);
            }
        }
        
        if (instr.opcode == OpCode::PUSH_CONST) {
            if (std::holds_alternative<Label>(instr.operand)) {
                int64_t val = std::get<Label>(instr.operand).index;
                if (val >= 0 && val < (int64_t)code.size()) {
                    next.push_back((size_t)val);
                }
            }
        }

        if (instr.opcode != OpCode::JUMP && instr.opcode != OpCode::RETURN && instr.opcode != OpCode::HALT) {
            if (i + 1 < code.size()) next.push_back(i + 1);
        }

        for (size_t n : next) {
            if (n < code.size() && !reachable[n]) {
                reachable[n] = true;
                worklist.push(n);
            }
        }
    }

    std::vector<Instruction> result;
    std::vector<size_t> old_to_new(code.size(), 0);

    for (size_t i = 0; i < code.size(); ++i) {
        if (reachable[i] && code[i].opcode != OpCode::NOP) {
            old_to_new[i] = result.size();
            result.push_back(code[i]);
        }
    }

    // Second pass to properly map old targets to new targets
    std::vector<size_t> target_map(code.size());
    size_t current_new = result.size();
    for (int i = (int)code.size() - 1; i >= 0; --i) {
        if (reachable[i] && code[i].opcode != OpCode::NOP) {
            current_new = old_to_new[i];
        }
        target_map[i] = current_new;
    }

    // Patch jump targets
    for (auto& instr : result) {
        if (instr.opcode == OpCode::JUMP || instr.opcode == OpCode::JUMP_IF_FALSE || 
            instr.opcode == OpCode::CALL || instr.opcode == OpCode::PUSH_CONST) {
            if (std::holds_alternative<Label>(instr.operand)) {
                int64_t old_target = std::get<Label>(instr.operand).index;
                if (old_target >= 0 && old_target < (int64_t)target_map.size()) {
                    instr.operand = Label{(int64_t)target_map[old_target]};
                }
            }
        }
    }

    return result;
}

} // namespace vyronix