#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include "IR.hpp"

namespace vyronix {

enum class NativeType {
    NIL,
    INT,
    FLOAT,
    STRING,
    BOOL,
    STRUCT,
    ARRAY,
    ANY
};

using NativeFn = std::function<IRValue(const std::vector<IRValue>&)>;

struct NativeFunction {
    std::string name;
    std::vector<NativeType> parameterTypes;
    NativeFn implementation;
    bool isVariadic = false;
};

class NativeRegistry {
public:
    static NativeRegistry& getInstance();

    void registerFunction(const std::string& name, const std::vector<NativeType>& params, NativeFn impl, bool variadic = false);
    IRValue call(const std::string& name, const std::vector<IRValue>& args);
    bool exists(const std::string& name) const;

    void setupStdLib();

    // Experimental: FFI support
    void loadLibrary(const std::string& path);

private:
    NativeRegistry() = default;
    std::unordered_map<std::string, NativeFunction> functions_;
    std::vector<void*> loaded_libs_; // To store handles for cleanup

    NativeType getTypeOf(const IRValue& val) const;
    std::string typeToString(NativeType type) const;
};

} // namespace vyronix
