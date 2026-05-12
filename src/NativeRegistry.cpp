#include "vyronix/NativeRegistry.hpp"
#include "vyronix/VM.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <random>
#include <ctime>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace vyronix {

NativeRegistry& NativeRegistry::getInstance() {
    static NativeRegistry instance;
    return instance;
}

void NativeRegistry::registerFunction(const std::string& name, const std::vector<NativeType>& params, NativeFn impl, bool variadic) {
    functions_[name] = {name, params, std::move(impl), variadic};
}

IRValue NativeRegistry::call(const std::string& name, const std::vector<IRValue>& args) {
    auto it = functions_.find(name);
    if (it == functions_.end()) {
        throw std::runtime_error("Native function not found: " + name);
    }

    const auto& func = it->second;
    if (!func.isVariadic && args.size() != func.parameterTypes.size()) {
        throw std::runtime_error("Invalid argument count for " + name);
    }

    // TODO: Strict type checking of arguments
    return func.implementation(args);
}

bool NativeRegistry::exists(const std::string& name) const {
    return functions_.count(name) > 0;
}

void NativeRegistry::setupStdLib() {
    if (!functions_.empty()) return; // Already setup

    // --- IO Namespace ---
    auto io_print = [](const std::vector<IRValue>& args) -> IRValue {
        std::cout << VirtualMachine::valueToString(args[0]) << std::endl;
        return std::monostate{};
    };
    registerFunction("io.print", {NativeType::ANY}, io_print);

    auto io_input = [](const std::vector<IRValue>&) -> IRValue {
        std::string s;
        std::getline(std::cin, s);
        return s;
    };
    registerFunction("io.input", {}, io_input);

    // --- FS Namespace ---
    auto fs_read = [](const std::vector<IRValue>& args) -> IRValue {
        std::ifstream file(std::get<std::string>(args[0]));
        if (!file) return std::monostate{};
        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    };
    registerFunction("fs.read", {NativeType::STRING}, fs_read);

    auto fs_write = [](const std::vector<IRValue>& args) -> IRValue {
        std::ofstream file(std::get<std::string>(args[0]));
        if (!file) return false;
        file << std::get<std::string>(args[1]);
        return true;
    };
    registerFunction("fs.write", {NativeType::STRING, NativeType::STRING}, fs_write);

    // --- String Namespace ---
    auto str_len = [](const std::vector<IRValue>& args) -> IRValue {
        if (std::holds_alternative<std::string>(args[0])) return (int64_t)std::get<std::string>(args[0]).length();
        if (std::holds_alternative<std::shared_ptr<ArrayInstance>>(args[0])) return (int64_t)std::get<std::shared_ptr<ArrayInstance>>(args[0])->elements.size();
        return (int64_t)0;
    };
    registerFunction("str.len", {NativeType::ANY}, str_len);

    auto str_upper = [](const std::vector<IRValue>& args) -> IRValue {
        std::string s = std::get<std::string>(args[0]);
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
        return s;
    };
    registerFunction("str.upper", {NativeType::STRING}, str_upper);

    auto str_lower = [](const std::vector<IRValue>& args) -> IRValue {
        std::string s = std::get<std::string>(args[0]);
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    };
    registerFunction("str.lower", {NativeType::STRING}, str_lower);

    auto str_split = [](const std::vector<IRValue>& args) -> IRValue {
        std::string s = std::get<std::string>(args[0]);
        std::string delim = std::get<std::string>(args[1]);
        auto arr = std::make_shared<ArrayInstance>();
        size_t pos = 0;
        while ((pos = s.find(delim)) != std::string::npos) {
            arr->elements.push_back(s.substr(0, pos));
            s.erase(0, pos + delim.length());
        }
        arr->elements.push_back(s);
        return arr;
    };
    registerFunction("str.split", {NativeType::STRING, NativeType::STRING}, str_split);

    // --- Array Namespace ---
    registerFunction("arr.push", {NativeType::ARRAY, NativeType::ANY}, [](const std::vector<IRValue>& args) -> IRValue {
        std::get<std::shared_ptr<ArrayInstance>>(args[0])->elements.push_back(args[1]);
        return args[1];
    });

    registerFunction("arr.pop", {NativeType::ARRAY}, [](const std::vector<IRValue>& args) -> IRValue {
        auto& elms = std::get<std::shared_ptr<ArrayInstance>>(args[0])->elements;
        if (elms.empty()) return std::monostate{};
        auto val = elms.back();
        elms.pop_back();
        return val;
    });

    registerFunction("arr.sort", {NativeType::ARRAY}, [](const std::vector<IRValue>& args) -> IRValue {
        auto& elms = std::get<std::shared_ptr<ArrayInstance>>(args[0])->elements;
        std::sort(elms.begin(), elms.end(), [](const IRValue& a, const IRValue& b) {
            if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
                return std::get<int64_t>(a) < std::get<int64_t>(b);
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
                return std::get<double>(a) < std::get<double>(b);
            if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b))
                return std::get<std::string>(a) < std::get<std::string>(b);
            return false;
        });
        return args[0];
    });

    // --- Math Namespace ---
    auto math_sqrt = [](const std::vector<IRValue>& args) -> IRValue {
        double d = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : (double)std::get<int64_t>(args[0]);
        return std::sqrt(d);
    };
    registerFunction("math.sqrt", {NativeType::ANY}, math_sqrt);

    auto math_pow = [](const std::vector<IRValue>& args) -> IRValue {
        double b = std::holds_alternative<double>(args[0]) ? std::get<double>(args[0]) : (double)std::get<int64_t>(args[0]);
        double e = std::holds_alternative<double>(args[1]) ? std::get<double>(args[1]) : (double)std::get<int64_t>(args[1]);
        return std::pow(b, e);
    };
    registerFunction("math.pow", {NativeType::ANY, NativeType::ANY}, math_pow);

    auto math_random = [](const std::vector<IRValue>&) -> IRValue {
        static std::mt19937 gen(std::random_device{}());
        static std::uniform_real_distribution<> dis(0, 1.0);
        return dis(gen);
    };
    registerFunction("math.random", {}, math_random);

    // --- Sys Namespace ---
    auto sys_time = [](const std::vector<IRValue>&) -> IRValue {
        return (int64_t)std::time(nullptr);
    };
    registerFunction("sys.time", {}, sys_time);

    auto sys_exit = [](const std::vector<IRValue>& args) -> IRValue {
        std::exit((int)std::get<int64_t>(args[0]));
        return std::monostate{};
    };
    registerFunction("sys.exit", {NativeType::INT}, sys_exit);

    // --- Weak Namespace ---
    registerFunction("weak.create", {NativeType::ANY}, [](const std::vector<IRValue>& args) -> IRValue {
        if (std::holds_alternative<std::shared_ptr<StructInstance>>(args[0])) {
            return WeakPointer{std::get<std::shared_ptr<StructInstance>>(args[0])};
        } else if (std::holds_alternative<std::shared_ptr<ArrayInstance>>(args[0])) {
            return WeakPointer{std::get<std::shared_ptr<ArrayInstance>>(args[0])};
        } else if (std::holds_alternative<std::shared_ptr<Closure>>(args[0])) {
            return WeakPointer{std::get<std::shared_ptr<Closure>>(args[0])};
        }
        return std::monostate{};
    });

    registerFunction("weak.lock", {NativeType::ANY}, [](const std::vector<IRValue>& args) -> IRValue {
        if (!std::holds_alternative<WeakPointer>(args[0])) return std::monostate{};
        auto& wp = std::get<WeakPointer>(args[0]);
        if (wp.kind == WeakPointer::Kind::STRUCT) {
            auto shared = wp.s_ptr.lock();
            if (shared) return shared;
        } else if (wp.kind == WeakPointer::Kind::ARRAY) {
            auto shared = wp.a_ptr.lock();
            if (shared) return shared;
        } else if (wp.kind == WeakPointer::Kind::CLOSURE) {
            auto shared = wp.c_ptr.lock();
            if (shared) return shared;
        }
        return IRValue{std::monostate{}};
    });

    registerFunction("weak.valid", {NativeType::ANY}, [](const std::vector<IRValue>& args) -> IRValue {
        if (!std::holds_alternative<WeakPointer>(args[0])) return false;
        auto& wp = std::get<WeakPointer>(args[0]);
        if (wp.kind == WeakPointer::Kind::STRUCT) return !wp.s_ptr.expired();
        if (wp.kind == WeakPointer::Kind::ARRAY) return !wp.a_ptr.expired();
        if (wp.kind == WeakPointer::Kind::CLOSURE) return !wp.c_ptr.expired();
        return false;
    });

    // --- Global / Legacy Support ---
    // (Keeping some for backward compatibility if needed, or remove if strict)
    registerFunction("print", {NativeType::ANY}, io_print);
    registerFunction("assert", {NativeType::BOOL}, [](const std::vector<IRValue>& args) -> IRValue {
        if (!std::get<bool>(args[0])) throw std::runtime_error("Assertion failed");
        return true;
    });
    registerFunction("log", {NativeType::ANY}, [](const std::vector<IRValue>& args) -> IRValue {
        std::cout << "[LOG] " << VirtualMachine::valueToString(args[0]) << std::endl;
        return std::monostate{};
    }, true);

    registerFunction("loadLibrary", {NativeType::STRING}, [](const std::vector<IRValue>& args) -> IRValue {
        NativeRegistry::getInstance().loadLibrary(std::get<std::string>(args[0]));
        return true;
    });

    registerFunction("system", {NativeType::STRING}, [](const std::vector<IRValue>& args) -> IRValue {
        return (int64_t)std::system(std::get<std::string>(args[0]).c_str());
    });

    registerFunction("sort", {NativeType::ARRAY}, [](const std::vector<IRValue>& args) -> IRValue {
        auto arr = std::get<std::shared_ptr<ArrayInstance>>(args[0]);
        std::sort(arr->elements.begin(), arr->elements.end(), [](const IRValue& a, const IRValue& b) {
            if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b))
                return std::get<int64_t>(a) < std::get<int64_t>(b);
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b))
                return std::get<double>(a) < std::get<double>(b);
            if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b))
                return std::get<std::string>(a) < std::get<std::string>(b);
            return false;
        });
        return args[0];
    });
}

void NativeRegistry::loadLibrary(const std::string& path) {
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) throw std::runtime_error("Could not load library: " + path);
    loaded_libs_.push_back((void*)handle);
#else
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) throw std::runtime_error("Could not load library: " + dlerror());
    loaded_libs_.push_back(handle);
#endif
}

NativeType NativeRegistry::getTypeOf(const IRValue& val) const {
    if (std::holds_alternative<std::monostate>(val)) return NativeType::NIL;
    if (std::holds_alternative<int64_t>(val)) return NativeType::INT;
    if (std::holds_alternative<double>(val)) return NativeType::FLOAT;
    if (std::holds_alternative<std::string>(val)) return NativeType::STRING;
    if (std::holds_alternative<bool>(val)) return NativeType::BOOL;
    if (std::holds_alternative<std::shared_ptr<StructInstance>>(val)) return NativeType::STRUCT;
    if (std::holds_alternative<std::shared_ptr<ArrayInstance>>(val)) return NativeType::ARRAY;
    return NativeType::ANY;
}

std::string NativeRegistry::typeToString(NativeType type) const {
    switch (type) {
        case NativeType::NIL: return "nil";
        case NativeType::INT: return "int";
        case NativeType::FLOAT: return "float";
        case NativeType::STRING: return "string";
        case NativeType::BOOL: return "bool";
        case NativeType::STRUCT: return "struct";
        case NativeType::ARRAY: return "array";
        case NativeType::ANY: return "any";
        default: return "unknown";
    }
}

} // namespace vyronix
