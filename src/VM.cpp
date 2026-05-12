#include "vyronix/VM.hpp"
#include "vyronix/Error.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iomanip>

namespace vyronix {

static bool epsilon_compare(double a, double b) {
    return std::abs(a - b) < 1e-9;
}

VirtualMachine::VirtualMachine() noexcept {
    setupStdLib();
}

VirtualMachine::~VirtualMachine() {
    // Break cycles before destruction
    globals_.clear();
    locals_stack_.clear();
    stack_.clear();
    while (!closure_context_.empty()) closure_context_.pop();
    
    for (auto& s : all_structs_) if (s) s->clear();
    for (auto& a : all_arrays_) if (a) a->clear();
    for (auto& c : all_closures_) if (c) c->clear();
    
    all_structs_.clear();
    all_arrays_.clear();
    all_closures_.clear();
}

void VirtualMachine::setupStdLib() noexcept {
    NativeRegistry::getInstance().setupStdLib();
}

void VirtualMachine::run(const std::vector<Instruction>& code) {
    ip_ = 0;
    while (ip_ < code.size()) {
        const auto& instr = code[ip_++];
        try {
            switch (instr.opcode) {
            case OpCode::PUSH_CONST:
                if (stack_.size() >= 1000) throw std::runtime_error("Stack overflow");
                push(instr.operand);
                break;
            case OpCode::LOAD_VAR: {
                std::string name = std::get<std::string>(instr.operand);
                if (!locals_stack_.empty() && locals_stack_.back().count(name)) {
                    push(locals_stack_.back()[name]);
                } else if (globals_.count(name)) {
                    push(globals_[name]);
                } else if (NativeRegistry::getInstance().exists(name)) {
                    push(name); // Push name as string for CALL to handle
                } else {
                    throw std::runtime_error("Undefined variable '" + name + "'");
                }
                break;
            }
            case OpCode::STORE_VAR: {
                checkStack(1);
                std::string name = std::get<std::string>(instr.operand);
                IRValue val = pop();
                if (!locals_stack_.empty()) {
                    locals_stack_.back()[name] = val;
                } else {
                    globals_[name] = val;
                }
                push(val); 
                break;
            }
            case OpCode::ADD: {
                checkStack(2);
                auto b = pop();
                auto a = pop();
                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b)) {
                    push(std::get<int64_t>(a) + std::get<int64_t>(b));
                } else if (std::holds_alternative<std::string>(a) || std::holds_alternative<std::string>(b)) {
                    push(valueToString(a) + valueToString(b));
                } else {
                    double da = std::holds_alternative<double>(a) ? std::get<double>(a) : static_cast<double>(std::get<int64_t>(a));
                    double db = std::holds_alternative<double>(b) ? std::get<double>(b) : static_cast<double>(std::get<int64_t>(b));
                    push(da + db);
                }
                break;
            }
            case OpCode::SUB: {
                checkStack(2);
                auto b = pop();
                auto a = pop();
                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b)) push(std::get<int64_t>(a) - std::get<int64_t>(b));
                else {
                    double da = std::holds_alternative<double>(a) ? std::get<double>(a) : static_cast<double>(std::get<int64_t>(a));
                    double db = std::holds_alternative<double>(b) ? std::get<double>(b) : static_cast<double>(std::get<int64_t>(b));
                    push(da - db);
                }
                break;
            }
            case OpCode::MUL: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b)) push(std::get<int64_t>(a) * std::get<int64_t>(b));
                else {
                    double da = std::holds_alternative<double>(a) ? std::get<double>(a) : static_cast<double>(std::get<int64_t>(a));
                    double db = std::holds_alternative<double>(b) ? std::get<double>(b) : static_cast<double>(std::get<int64_t>(b));
                    push(da * db);
                }
                break;
            }
            case OpCode::DIV: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b)) {
                    if (std::get<int64_t>(b) == 0) throw std::runtime_error("Division by zero");
                    push(std::get<int64_t>(a) / std::get<int64_t>(b));
                } else {
                    double db = std::holds_alternative<double>(b) ? std::get<double>(b) : static_cast<double>(std::get<int64_t>(b));
                    if (db == 0.0) throw std::runtime_error("Division by zero");
                    double da = std::holds_alternative<double>(a) ? std::get<double>(a) : static_cast<double>(std::get<int64_t>(a));
                    push(da / db);
                }
                break;
            }
            case OpCode::MOD: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                if (std::holds_alternative<int64_t>(a) && std::holds_alternative<int64_t>(b)) {
                    if (std::get<int64_t>(b) == 0) throw std::runtime_error("Modulo by zero");
                    push(std::get<int64_t>(a) % std::get<int64_t>(b));
                } else {
                    throw std::runtime_error("Modulo operator only supported for integers");
                }
                break;
            }
            case OpCode::EQ: {
                if (stack_.size() < 2) {
                    std::cerr << "[VM ERROR] EQ requires 2 operands, got " << stack_.size() << " at IP " << ip_ << std::endl;
                    throw std::runtime_error("Stack underflow");
                }
                auto b = pop();
                auto a = pop();
                push(a == b);
                break;
            }
            case OpCode::NE: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(a != b);
                break;
            }
            case OpCode::LT: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(a < b);
                break;
            }
            case OpCode::LE: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(a <= b);
                break;
            }
            case OpCode::GT: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(a > b);
                break;
            }
            case OpCode::GE: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(a >= b);
                break;
            }
            case OpCode::NOT: {
                if (stack_.size() < 1) throw std::runtime_error("Stack underflow");
                push(!isTrue(pop()));
                break;
            }
            case OpCode::AND: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(isTrue(a) && isTrue(b));
                break;
            }
            case OpCode::OR: {
                if (stack_.size() < 2) throw std::runtime_error("Stack underflow");
                auto b = pop();
                auto a = pop();
                push(isTrue(a) || isTrue(b));
                break;
            }
            case OpCode::JUMP: {
                if (std::holds_alternative<Label>(instr.operand)) {
                    ip_ = (size_t)std::get<Label>(instr.operand).index;
                } else {
                    ip_ = (size_t)std::get<int64_t>(instr.operand);
                }
                continue;
            }
            case OpCode::JUMP_IF_FALSE: {
                if (!isTrue(pop())) {
                    if (std::holds_alternative<Label>(instr.operand)) {
                        ip_ = (size_t)std::get<Label>(instr.operand).index;
                    } else {
                        ip_ = (size_t)std::get<int64_t>(instr.operand);
                    }
                    continue;
                }
                break;
            }
            case OpCode::CALL: {
                int64_t arg_count = std::get<int64_t>(instr.operand);
                if (stack_.empty()) {
                    std::cerr << "[VM ERROR] CALL requires callee on stack at IP " << ip_ << std::endl;
                    throw std::runtime_error("Stack underflow");
                }
                auto callee = pop();
                if (stack_.size() < (size_t)arg_count) {
                    std::cerr << "[VM ERROR] CALL requires " << arg_count << " args, got " << stack_.size() << " at IP " << ip_ << std::endl;
                    throw std::runtime_error("Stack underflow");
                }
                std::vector<IRValue> args(arg_count);
                for (int i = arg_count - 1; i >= 0; --i) args[i] = pop();
                
                if (std::holds_alternative<std::string>(callee)) {
                    push(NativeRegistry::getInstance().call(std::get<std::string>(callee), args));
                } else if (std::holds_alternative<Label>(callee)) {
                    call_stack_.push(ip_);
                    locals_stack_.push_back({});
                    for (auto& arg : args) push(arg);
                    ip_ = (size_t)std::get<Label>(callee).index;
                    continue;
                } else if (std::holds_alternative<int64_t>(callee)) {
                    call_stack_.push(ip_);
                    locals_stack_.push_back({});
                    for (auto& arg : args) push(arg);
                    ip_ = (size_t)std::get<int64_t>(callee);
                    continue;
                } else if (std::holds_alternative<std::shared_ptr<Closure>>(callee)) {
                    auto closure = std::get<std::shared_ptr<Closure>>(callee);
                    call_stack_.push(ip_);
                    locals_stack_.push_back({});
                    closure_context_.push(closure);
                    for (auto& arg : args) push(arg);
                    ip_ = closure->ip;
                    continue;
                }
                break;
            }
            case OpCode::RETURN: {
                if (call_stack_.empty()) return;
                
                // Close upvalues pointing to this frame
                auto& current_locals = locals_stack_.back();
                for (auto it = open_upvalues_.begin(); it != open_upvalues_.end(); ) {
                    bool closed_some = false;
                    for (auto& [name, val_ptr] : current_locals) {
                        if ((*it)->location == &val_ptr) {
                            (*it)->closed = std::move(val_ptr);
                            (*it)->is_closed = true;
                            (*it)->location = &((*it)->closed);
                            it = open_upvalues_.erase(it);
                            closed_some = true;
                            break;
                        }
                    }
                    if (!closed_some) ++it;
                }

                ip_ = call_stack_.top();
                call_stack_.pop();
                locals_stack_.pop_back();
                if (!closure_context_.empty()) closure_context_.pop();
                break;
            }
            case OpCode::MAKE_CLOSURE: {
                int64_t capture_count = std::get<int64_t>(instr.operand);
                std::vector<std::pair<bool, IRValue>> capture_info(capture_count);
                
                // Capture descriptions are on stack in reverse order of IR emission
                // IR emitted: [is_local, index_or_name] for each capture
                // So stack has: [..., body_ip, is_local1, info1, is_local2, info2, ...]
                // We need to pop them from last to first
                for (int i = capture_count - 1; i >= 0; --i) {
                    auto info = pop();
                    auto is_local = std::get<bool>(pop());
                    capture_info[i] = {is_local, std::move(info)};
                }
                
                auto body_val = pop();
                size_t body_ip;
                if (std::holds_alternative<Label>(body_val)) {
                    body_ip = (size_t)std::get<Label>(body_val).index;
                } else {
                    body_ip = (size_t)std::get<int64_t>(body_val);
                }
                auto closure = std::make_shared<Closure>(body_ip);
                all_closures_.push_back(closure);
                
                for (const auto& [is_local, info] : capture_info) {
                    if (is_local) {
                        std::string var_name = std::get<std::string>(info);
                        // Find the value in the current frame
                        auto& current_locals = locals_stack_.back();
                        if (current_locals.find(var_name) == current_locals.end()) {
                            // If not found, it might be an uninitialized local. 
                            // Let's create it.
                            current_locals[var_name] = std::monostate{};
                        }
                        
                        IRValue* loc = &current_locals[var_name];
                        
                        // Check if we already have an open upvalue for this location
                        std::shared_ptr<Upvalue> upvalue = nullptr;
                        for (auto& open_uv : open_upvalues_) {
                            if (open_uv->location == loc) {
                                upvalue = open_uv;
                                break;
                            }
                        }
                        
                        if (!upvalue) {
                            upvalue = std::make_shared<Upvalue>(loc);
                            open_upvalues_.push_back(upvalue);
                        }
                        closure->upvalues.push_back(upvalue);
                    } else {
                        // Capture from parent closure's upvalues
                        int64_t upvalue_idx = std::get<int64_t>(info);
                        if (closure_context_.empty()) throw std::runtime_error("No closure context for upvalue capture");
                        closure->upvalues.push_back(closure_context_.top()->upvalues[upvalue_idx]);
                    }
                }
                
                push(closure);
                break;
            }
            case OpCode::LOAD_UPVALUE: {
                int64_t idx = std::get<int64_t>(instr.operand);
                if (closure_context_.empty()) throw std::runtime_error("No closure context for LOAD_UPVALUE");
                auto uv = closure_context_.top()->upvalues[idx];
                push(*(uv->location));
                break;
            }
            case OpCode::STORE_UPVALUE: {
                int64_t idx = std::get<int64_t>(instr.operand);
                if (closure_context_.empty()) throw std::runtime_error("No closure context for STORE_UPVALUE");
                auto uv = closure_context_.top()->upvalues[idx];
                IRValue val = pop();
                *(uv->location) = val;
                push(val);
                break;
            }
            case OpCode::HALT: return;
            case OpCode::GET_FIELD: {
                auto obj = pop();
                std::string field_name = std::get<std::string>(instr.operand);
                if (std::holds_alternative<std::shared_ptr<StructInstance>>(obj)) {
                    auto inst = std::get<std::shared_ptr<StructInstance>>(obj);
                    push(inst->fields[field_name]);
                } else if (std::holds_alternative<std::shared_ptr<ClassInstance>>(obj)) {
                    auto inst = std::get<std::shared_ptr<ClassInstance>>(obj);
                    if (inst->fields.count(field_name)) {
                        push(inst->fields[field_name]);
                    } else {
                        // Look in method table
                        auto curr_def = inst->definition;
                        while (curr_def) {
                            if (curr_def->methods.count(field_name)) {
                                push(obj); // Push 'self'
                                push((int64_t)curr_def->methods[field_name]);
                                goto found_field;
                            }
                            // Search superclass
                            if (!curr_def->superclass.empty()) {
                                auto it = globals_.find(curr_def->superclass);
                                if (it != globals_.end() && std::holds_alternative<std::shared_ptr<ClassDefinition>>(it->second)) {
                                    curr_def = std::get<std::shared_ptr<ClassDefinition>>(it->second);
                                    continue;
                                }
                            }
                            break;
                        }
                        throw std::runtime_error("Field/Method '" + field_name + "' not found in class " + inst->definition->name);
                    }
                }
                found_field:;
                break;
            }
            case OpCode::SET_FIELD: {
                auto val = pop();
                auto obj = pop();
                std::string field_name = std::get<std::string>(instr.operand);
                if (std::holds_alternative<std::shared_ptr<StructInstance>>(obj)) {
                    auto inst = std::get<std::shared_ptr<StructInstance>>(obj);
                    inst->fields[field_name] = val;
                    push(val);
                } else if (std::holds_alternative<std::shared_ptr<ClassInstance>>(obj)) {
                    auto inst = std::get<std::shared_ptr<ClassInstance>>(obj);
                    inst->fields[field_name] = val;
                    push(val);
                }
                break;
            }
            case OpCode::CLASS_DEF: {
                std::string data = std::get<std::string>(instr.operand);
                size_t colon = data.find(':');
                std::string name = data.substr(0, colon);
                std::string super = (colon != std::string::npos) ? data.substr(colon + 1) : "";

                auto def = std::make_shared<ClassDefinition>(name, super);
                
                // Inherit fields from superclass
                if (!super.empty()) {
                    auto it = globals_.find(super);
                    if (it != globals_.end() && std::holds_alternative<std::shared_ptr<ClassDefinition>>(it->second)) {
                        auto super_def = std::get<std::shared_ptr<ClassDefinition>>(it->second);
                        def->fields = super_def->fields;
                        def->methods = super_def->methods;
                    }
                }

                // Collect methods starting with "name."
                for (const auto& [lbl, val] : globals_) {
                    if (lbl.find(name + ".") == 0) {
                        std::string method_name = lbl.substr(name.length() + 1);
                        if (std::holds_alternative<int64_t>(val)) {
                            def->methods[method_name] = (size_t)std::get<int64_t>(val);
                        } else if (std::holds_alternative<Label>(val)) {
                            def->methods[method_name] = (size_t)std::get<Label>(val).index;
                        }
                    }
                }

                globals_[name] = def;
                all_class_definitions_.push_back(def);
                break;
            }
            case OpCode::NEW_INSTANCE: {
                std::string class_name = std::get<std::string>(instr.operand);
                if (globals_.count(class_name) && std::holds_alternative<std::shared_ptr<ClassDefinition>>(globals_[class_name])) {
                    auto def = std::get<std::shared_ptr<ClassDefinition>>(globals_[class_name]);
                    auto inst = std::make_shared<ClassInstance>(def);
                    inst->fields = def->fields;
                    push(inst);
                    all_class_instances_.push_back(inst);
                } else {
                    throw std::runtime_error("Unknown class '" + class_name + "'");
                }
                break;
            }
            case OpCode::NEW_STRUCT: {
                auto inst = std::make_shared<StructInstance>();
                all_structs_.push_back(inst);
                int64_t count = std::get<int64_t>(instr.operand);
                for (int i = 0; i < count; ++i) {
                    std::string name = std::get<std::string>(pop());
                    inst->fields[name] = pop();
                }
                push(inst);
                break;
            }
            case OpCode::GET_INDEX: {
                auto idx = std::get<int64_t>(pop());
                auto obj = pop();
                if (std::holds_alternative<std::string>(obj)) push(std::string(1, std::get<std::string>(obj)[idx]));
                else push(std::get<std::shared_ptr<ArrayInstance>>(obj)->elements[idx]);
                break;
            }
            case OpCode::SET_INDEX: {
                auto val = pop();
                auto idx = std::get<int64_t>(pop());
                auto arr = std::get<std::shared_ptr<ArrayInstance>>(pop());
                arr->elements[idx] = val;
                push(val);
                break;
            }
            case OpCode::GET_SLICE: {
                std::cout << "[DEBUG] GET_SLICE stack_size=" << stack_.size() << std::endl;
                if (stack_.size() < 3) throw std::runtime_error("Stack underflow in GET_SLICE");
                auto end_val = pop();
                auto start_val = pop();
                auto obj = pop();
                
                int64_t end = std::get<int64_t>(end_val);
                int64_t start = std::get<int64_t>(start_val);

                if (std::holds_alternative<std::string>(obj)) {
                    std::string s = std::get<std::string>(obj);
                    if (end == -1) end = s.length();
                    push(s.substr(start, end - start));
                } else if (std::holds_alternative<std::shared_ptr<ArrayInstance>>(obj)) {
                    auto arr = std::get<std::shared_ptr<ArrayInstance>>(obj);
                    if (end == -1) end = arr->elements.size();
                    auto new_arr = std::make_shared<ArrayInstance>();
                    all_arrays_.push_back(new_arr);
                    for (int64_t i = start; i < end; ++i) {
                        if (i >= 0 && i < (int64_t)arr->elements.size()) {
                            new_arr->elements.push_back(arr->elements[i]);
                        }
                    }
                    push(new_arr);
                } else {
                    throw std::runtime_error("Cannot slice non-array/string");
                }
                break;
            }
            case OpCode::NEW_ARRAY: {
                auto arr = std::make_shared<ArrayInstance>();
                all_arrays_.push_back(arr);
                int64_t size = std::get<int64_t>(instr.operand);
                for (int i = 0; i < size; ++i) arr->elements.insert(arr->elements.begin(), pop());
                push(arr);
                break;
            }
            case OpCode::TRY_BEGIN: {
                exception_handlers_.push_back({(size_t)std::get<int64_t>(instr.operand), stack_.size(), ""});
                break;
            }
            case OpCode::TRY_END: {
                exception_handlers_.pop_back();
                break;
            }
            case OpCode::THROW: {
                throw std::runtime_error(valueToString(pop()));
            }
            case OpCode::POP: 
                if (stack_.empty()) {
                    std::cerr << "[VM ERROR] POP on empty stack at IP " << ip_ << std::endl;
                    throw std::runtime_error("Stack underflow");
                }
                (void)pop(); 
                break;
            case OpCode::DUP: 
                if (stack_.empty()) {
                    std::cerr << "[VM ERROR] DUP on empty stack at IP " << ip_ << std::endl;
                    throw std::runtime_error("Stack underflow");
                }
                push(stack_.back()); 
                break;
            case OpCode::NOP: break;
            }
        } catch (const std::exception& e) {
            if (!exception_handlers_.empty()) {
                auto handler = exception_handlers_.back();
                exception_handlers_.pop_back();
                while (stack_.size() > handler.stack_depth) (void)pop();
                push(std::string(e.what()));
                ip_ = handler.catch_ip;
            } else {
                throw;
            }
        }
    }
}

std::string VirtualMachine::valueToString(const IRValue& val) {
    if (std::holds_alternative<std::monostate>(val)) return "null";
    if (std::holds_alternative<int64_t>(val)) return std::to_string(std::get<int64_t>(val));
    if (std::holds_alternative<double>(val)) return std::to_string(std::get<double>(val));
    if (std::holds_alternative<std::string>(val)) return std::get<std::string>(val);
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val) ? "true" : "false";
    if (std::holds_alternative<Label>(val)) return "@" + std::to_string(std::get<Label>(val).index);
    if (std::holds_alternative<WeakPointer>(val)) return "[weak pointer]";
    if (std::holds_alternative<std::shared_ptr<StructInstance>>(val)) return "[struct instance]";
    if (std::holds_alternative<std::shared_ptr<Closure>>(val)) return "[closure]";
    if (std::holds_alternative<std::shared_ptr<Upvalue>>(val)) return "[upvalue]";
    if (std::holds_alternative<std::shared_ptr<ClassDefinition>>(val)) return "[class " + std::get<std::shared_ptr<ClassDefinition>>(val)->name + "]";
    if (std::holds_alternative<std::shared_ptr<ClassInstance>>(val)) return "[instance of " + std::get<std::shared_ptr<ClassInstance>>(val)->definition->name + "]";
    if (std::holds_alternative<std::shared_ptr<ArrayInstance>>(val)) {
        auto arr = std::get<std::shared_ptr<ArrayInstance>>(val);
        std::string s = "[";
        for (size_t i = 0; i < arr->elements.size(); ++i) {
            s += valueToString(arr->elements[i]);
            if (i < arr->elements.size() - 1) s += ", ";
        }
        s += "]";
        return s;
    }
    return "unknown";
}

std::string VirtualMachine::opcodeToString(OpCode op) noexcept {
    switch (op) {
        case OpCode::PUSH_CONST: return "PUSH_CONST";
        case OpCode::LOAD_VAR: return "LOAD_VAR";
        case OpCode::STORE_VAR: return "STORE_VAR";
        case OpCode::ADD: return "ADD";
        case OpCode::SUB: return "SUB";
        case OpCode::MUL: return "MUL";
        case OpCode::DIV: return "DIV";
        case OpCode::EQ: return "EQ";
        case OpCode::NE: return "NE";
        case OpCode::LT: return "LT";
        case OpCode::LE: return "LE";
        case OpCode::GT: return "GT";
        case OpCode::GE: return "GE";
        case OpCode::AND: return "AND";
        case OpCode::OR: return "OR";
        case OpCode::NOT: return "NOT";
        case OpCode::JUMP: return "JUMP";
        case OpCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case OpCode::CALL: return "CALL";
        case OpCode::RETURN: return "RETURN";
        case OpCode::HALT: return "HALT";
        case OpCode::GET_FIELD: return "GET_FIELD";
        case OpCode::SET_FIELD: return "SET_FIELD";
        case OpCode::NEW_STRUCT: return "NEW_STRUCT";
        case OpCode::GET_INDEX: return "GET_INDEX";
        case OpCode::SET_INDEX: return "SET_INDEX";
        case OpCode::GET_SLICE: return "GET_SLICE";
        case OpCode::NEW_ARRAY: return "NEW_ARRAY";
        case OpCode::MAKE_CLOSURE: return "MAKE_CLOSURE";
        case OpCode::LOAD_UPVALUE: return "LOAD_UPVALUE";
        case OpCode::STORE_UPVALUE: return "STORE_UPVALUE";
        case OpCode::TRY_BEGIN: return "TRY_BEGIN";
        case OpCode::TRY_END: return "TRY_END";
        case OpCode::THROW: return "THROW";
        case OpCode::CLASS_DEF: return "CLASS_DEF";
        case OpCode::NEW_INSTANCE: return "NEW_INSTANCE";
        case OpCode::POP: return "POP";
        case OpCode::DUP: return "DUP";
        case OpCode::NOP: return "NOP";
        default: return "UNKNOWN";
    }
}

void VirtualMachine::disassemble(const std::vector<Instruction>& code) noexcept {
    for (size_t i = 0; i < code.size(); ++i) {
        std::cout << std::setw(4) << i << ": " << std::setw(15) << std::left << opcodeToString(code[i].opcode) << " ";
        std::cout << valueToString(code[i].operand) << "\n";
    }
}

void VirtualMachine::dumpState(const std::vector<Instruction>& code) const noexcept {
    std::cout << "--- VM STATE DUMP ---\n";
    std::cout << "IP: " << ip_ << " (";
    if (ip_ > 0 && ip_ <= code.size()) {
        std::cout << opcodeToString(code[ip_ - 1].opcode);
    } else {
        std::cout << "INVALID";
    }
    std::cout << ")\n";
    std::cout << "Stack Size: " << stack_.size() << "\n";
    for (int i = (int)stack_.size() - 1; i >= std::max(0, (int)stack_.size() - 5); --i) {
        std::cout << "  [" << i << "]: " << valueToString(stack_[i]) << "\n";
    }
    std::cout << "Call Depth: " << call_stack_.size() << "\n";
    std::cout << "---------------------\n";
}

bool VirtualMachine::isTrue(const IRValue& val) noexcept {
    if (std::holds_alternative<bool>(val)) return std::get<bool>(val);
    if (std::holds_alternative<std::monostate>(val)) return false;
    if (std::holds_alternative<int64_t>(val)) return std::get<int64_t>(val) != 0;
    if (std::holds_alternative<double>(val)) return !epsilon_compare(std::get<double>(val), 0.0);
    if (std::holds_alternative<std::string>(val)) return !std::get<std::string>(val).empty();
    return true;
}

} // namespace vyronix