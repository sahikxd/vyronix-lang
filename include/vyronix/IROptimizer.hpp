#pragma once

#include <vector>
#include "IR.hpp"

namespace vyronix {

class IROptimizer {
public:
    std::vector<Instruction> optimize(const std::vector<Instruction>& code);

private:
    std::vector<Instruction> constantFolding(const std::vector<Instruction>& code);
    std::vector<Instruction> peephole(const std::vector<Instruction>& code);
    std::vector<Instruction> deadCodeElimination(const std::vector<Instruction>& code);
};

} // namespace vyronix
