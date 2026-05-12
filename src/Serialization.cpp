#include "vyronix/Serialization.hpp"
#include <stdexcept>
#include <cstring>

namespace vyronix {

bool Serializer::serialize(const std::vector<Instruction>& code, const std::string& filename) {
    std::ofstream os(filename, std::ios::binary);
    if (!os.is_open()) return false;

    // Header
    os.write(reinterpret_cast<const char*>(&MAGIC), sizeof(MAGIC));
    os.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));
    
    // Placeholder for checksum
    uint32_t checksum = 0;
    os.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));

    uint64_t size = code.size();
    os.write(reinterpret_cast<const char*>(&size), sizeof(size));

    for (const auto& inst : code) {
        uint8_t op = static_cast<uint8_t>(inst.opcode);
        os.write(reinterpret_cast<const char*>(&op), sizeof(op));
        writeValue(os, inst.operand);
        os.write(reinterpret_cast<const char*>(&inst.line), sizeof(inst.line));
        os.write(reinterpret_cast<const char*>(&inst.column), sizeof(inst.column));
    }

    return true;
}

std::vector<Instruction> Serializer::deserialize(const std::string& filename) {
    std::ifstream is(filename, std::ios::binary);
    if (!is.is_open()) throw std::runtime_error("Could not open bytecode file: " + filename);

    uint32_t magic, version, checksum;
    is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    is.read(reinterpret_cast<char*>(&version), sizeof(version));
    is.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));

    if (magic != MAGIC) throw std::runtime_error("Invalid bytecode magic (Expected VYXB)");
    if (version != VERSION) throw std::runtime_error("Incompatible bytecode version");

    uint64_t size;
    is.read(reinterpret_cast<char*>(&size), sizeof(size));

    std::vector<Instruction> code;
    code.reserve(size);

    for (uint64_t i = 0; i < size; ++i) {
        uint8_t op;
        is.read(reinterpret_cast<char*>(&op), sizeof(op));
        IRValue val = readValue(is);
        uint32_t line, col;
        is.read(reinterpret_cast<char*>(&line), sizeof(line));
        is.read(reinterpret_cast<char*>(&col), sizeof(col));
        code.emplace_back(static_cast<OpCode>(op), std::move(val), line, col);
    }

    return code;
}

void Serializer::writeValue(std::ostream& os, const IRValue& value) {
    uint8_t type = static_cast<uint8_t>(value.index());
    os.write(reinterpret_cast<const char*>(&type), sizeof(type));

    if (std::holds_alternative<int64_t>(value)) {
        int64_t v = std::get<int64_t>(value);
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    } else if (std::holds_alternative<double>(value)) {
        double v = std::get<double>(value);
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    } else if (std::holds_alternative<std::string>(value)) {
        const std::string& v = std::get<std::string>(value);
        uint64_t len = v.length();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(v.data(), len);
    } else if (std::holds_alternative<bool>(value)) {
        bool v = std::get<bool>(value);
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    } else if (std::holds_alternative<Label>(value)) {
        int64_t v = std::get<Label>(value).index;
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }
    // std::monostate, shared_ptrs are not serialized (not constants)
}

IRValue Serializer::readValue(std::istream& is) {
    uint8_t type;
    is.read(reinterpret_cast<char*>(&type), sizeof(type));

    switch (type) {
        case 0: return std::monostate{};
        case 1: {
            int64_t v;
            is.read(reinterpret_cast<char*>(&v), sizeof(v));
            return v;
        }
        case 2: {
            double v;
            is.read(reinterpret_cast<char*>(&v), sizeof(v));
            return v;
        }
        case 3: {
            uint64_t len;
            is.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string v(len, '\0');
            is.read(&v[0], len);
            return v;
        }
        case 4: {
            bool v;
            is.read(reinterpret_cast<char*>(&v), sizeof(v));
            return v;
        }
        case 5: { // Label
            int64_t v;
            is.read(reinterpret_cast<char*>(&v), sizeof(v));
            return Label{v};
        }
        default: return std::monostate{};
    }
}

} // namespace vyronix
