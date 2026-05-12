#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "IR.hpp"

namespace vyronix {

class Serializer {
public:
    static bool serialize(const std::vector<Instruction>& code, const std::string& filename);
    static std::vector<Instruction> deserialize(const std::string& filename);

private:
    static void writeValue(std::ostream& os, const IRValue& value);
    static IRValue readValue(std::istream& is);
    
    static constexpr uint32_t MAGIC = 0x56595842; // "VYXB"
    static constexpr uint32_t VERSION = 1;
};

} // namespace vyronix
