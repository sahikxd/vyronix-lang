#pragma once

#include <vector>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include "IR.hpp"
#include "NativeRegistry.hpp"
#include "Error.hpp"

#include <functional>
#include <string>

namespace vyronix {

/**
 * OWNERSHIP RULES & MEMORY POLICY
 * -------------------------------
 * 1. The VM uses std::shared_ptr for managed objects (StructInstance, ArrayInstance).
 * 2. IRValue is a variant that can hold these shared_ptrs.
 * 3. AVOID CIRCULAR REFERENCES: Structs/Arrays should not hold shared_ptrs to themselves 
 *    or their parents without careful consideration. For now, VYRONIX does not have 
 *    a cycle-detecting GC, so leaked cycles will persist until VM shutdown.
 * 4. PERFORMANCE NOTE: shared_ptr involves atomic refcounting. For high-performance 
 *    hot paths, consider alternative memory management in future versions (v2+).
 */

using NativeFn = std::function<IRValue(const std::vector<IRValue>&)>;

class VirtualMachine {
public:
    VirtualMachine() noexcept;
    ~VirtualMachine();
    void run(const std::vector<Instruction>& code);
    
    [[nodiscard]] static std::string valueToString(const IRValue& val);
    [[nodiscard]] static std::string opcodeToString(OpCode op) noexcept;
    static void disassemble(const std::vector<Instruction>& code) noexcept;

    void dumpState(const std::vector<Instruction>& code) const noexcept;
    void setupStdLib() noexcept;

private:
    struct ExceptionHandler {
        size_t catch_ip;
        size_t stack_depth;
        std::string catch_var;
    };

    std::vector<IRValue> stack_;
    std::unordered_map<std::string, IRValue> globals_;
    std::stack<size_t> call_stack_;
    std::vector<std::unordered_map<std::string, IRValue>> locals_stack_;
    std::stack<std::shared_ptr<Closure>> closure_context_;
    std::vector<std::shared_ptr<Upvalue>> open_upvalues_;
    std::vector<ExceptionHandler> exception_handlers_;
    size_t ip_ = 0;
    
    // Cycle breaking tracking
    std::vector<std::shared_ptr<StructInstance>> all_structs_;
    std::vector<std::shared_ptr<ArrayInstance>> all_arrays_;
    std::vector<std::shared_ptr<Closure>> all_closures_;
    std::vector<std::shared_ptr<ClassInstance>> all_class_instances_;
    std::vector<std::shared_ptr<ClassDefinition>> all_class_definitions_;
    size_t call_depth_ = 0;
    const size_t max_call_depth_ = 500; // Prevent host stack overflow

    void push(IRValue val) { stack_.push_back(std::move(val)); }
    [[nodiscard]] IRValue pop() {
        if (stack_.empty()) {
            std::string msg = "Stack underflow at IP " + std::to_string(ip_);
            throw RuntimeError(msg, "unknown", 0, 0);
        }
        IRValue val = std::move(stack_.back());
        stack_.pop_back();
        return val;
    }

    void checkStack(size_t required) {
        if (stack_.size() < required) throw RuntimeError("Stack underflow (guard)", "unknown", 0, 0);
    }

    void checkJump(int64_t target, size_t code_size) {
        if (target < 0 || target >= (int64_t)code_size) {
            throw RuntimeError("Invalid jump target: " + std::to_string(target), "unknown", 0, 0);
        }
    }

    [[nodiscard]] bool isTrue(const IRValue& val) noexcept;
};

} // namespace vyronix
