#pragma once

#include <vector>
#include <string>
#include <variant>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <typeinfo>

namespace vyronix {

enum class OpCode : uint8_t {
    PUSH_CONST,
    LOAD_VAR,
    STORE_VAR,
    ADD, SUB, MUL, DIV, MOD,
    EQ, NE, LT, LE, GT, GE,
    AND, OR, NOT,
    JUMP, JUMP_IF_FALSE,
    CALL, RETURN, HALT,
    GET_FIELD, SET_FIELD, NEW_STRUCT,
    GET_INDEX, SET_INDEX, GET_SLICE, NEW_ARRAY,
    MAKE_CLOSURE, LOAD_UPVALUE, STORE_UPVALUE,
    TRY_BEGIN, TRY_END, THROW,
    CLASS_DEF, NEW_INSTANCE,
    POP, DUP,
    NOP
};

struct StructInstance;
struct ArrayInstance;
struct Closure;
struct Upvalue;
struct ClassInstance;
struct ClassDefinition;

struct WeakPointer {
    enum class Kind { STRUCT, ARRAY, CLOSURE, CLASS_INSTANCE } kind;
    union {
        std::weak_ptr<StructInstance> s_ptr;
        std::weak_ptr<ArrayInstance> a_ptr;
        std::weak_ptr<Closure> c_ptr;
        std::weak_ptr<ClassInstance> ci_ptr;
    };

    WeakPointer() : kind(Kind::STRUCT) { new(&s_ptr) std::weak_ptr<StructInstance>(); }
    ~WeakPointer() {
        if (kind == Kind::STRUCT) s_ptr.~weak_ptr();
        else if (kind == Kind::ARRAY) a_ptr.~weak_ptr();
        else if (kind == Kind::CLOSURE) c_ptr.~weak_ptr();
        else if (kind == Kind::CLASS_INSTANCE) ci_ptr.~weak_ptr();
    }
    WeakPointer(const WeakPointer& other) : kind(other.kind) {
        if (kind == Kind::STRUCT) new(&s_ptr) std::weak_ptr<StructInstance>(other.s_ptr);
        else if (kind == Kind::ARRAY) new(&a_ptr) std::weak_ptr<ArrayInstance>(other.a_ptr);
        else if (kind == Kind::CLOSURE) new(&c_ptr) std::weak_ptr<Closure>(other.c_ptr);
        else if (kind == Kind::CLASS_INSTANCE) new(&ci_ptr) std::weak_ptr<ClassInstance>(other.ci_ptr);
    }
    WeakPointer& operator=(const WeakPointer& other) {
        if (this != &other) {
            this->~WeakPointer();
            new(this) WeakPointer(other);
        }
        return *this;
    }

    explicit WeakPointer(const std::shared_ptr<StructInstance>& p) : kind(Kind::STRUCT) { new(&s_ptr) std::weak_ptr<StructInstance>(p); }
    explicit WeakPointer(const std::shared_ptr<ArrayInstance>& p) : kind(Kind::ARRAY) { new(&a_ptr) std::weak_ptr<ArrayInstance>(p); }
    explicit WeakPointer(const std::shared_ptr<Closure>& p) : kind(Kind::CLOSURE) { new(&c_ptr) std::weak_ptr<Closure>(p); }
    explicit WeakPointer(const std::shared_ptr<ClassInstance>& p) : kind(Kind::CLASS_INSTANCE) { new(&ci_ptr) std::weak_ptr<ClassInstance>(p); }

    bool operator==(const WeakPointer& other) const {
        if (kind != other.kind) return false;
        if (kind == Kind::STRUCT) return !s_ptr.owner_before(other.s_ptr) && !other.s_ptr.owner_before(s_ptr);
        if (kind == Kind::ARRAY) return !a_ptr.owner_before(other.a_ptr) && !other.a_ptr.owner_before(a_ptr);
        if (kind == Kind::CLOSURE) return !c_ptr.owner_before(other.c_ptr) && !other.c_ptr.owner_before(c_ptr);
        return !ci_ptr.owner_before(other.ci_ptr) && !other.ci_ptr.owner_before(ci_ptr);
    }
    bool operator!=(const WeakPointer& other) const { return !(*this == other); }
    bool operator<(const WeakPointer& other) const {
        if (kind != other.kind) return kind < other.kind;
        if (kind == Kind::STRUCT) return s_ptr.owner_before(other.s_ptr);
        if (kind == Kind::ARRAY) return a_ptr.owner_before(other.a_ptr);
        if (kind == Kind::CLOSURE) return c_ptr.owner_before(other.c_ptr);
        return ci_ptr.owner_before(other.ci_ptr);
    }
    bool operator>(const WeakPointer& other) const { return other < *this; }
    bool operator<=(const WeakPointer& other) const { return !(*this > other); }
    bool operator>=(const WeakPointer& other) const { return !(*this < other); }
};

struct Label {
    int64_t index;
    bool operator==(const Label& other) const { return index == other.index; }
    bool operator!=(const Label& other) const { return index != other.index; }
    bool operator<(const Label& other) const { return index < other.index; }
    bool operator>(const Label& other) const { return index > other.index; }
    bool operator<=(const Label& other) const { return index <= other.index; }
    bool operator>=(const Label& other) const { return index >= other.index; }
};

using IRValue = std::variant<
    std::monostate, 
    int64_t, 
    double, 
    std::string, 
    bool, 
    Label,
    WeakPointer,
    std::shared_ptr<StructInstance>,
    std::shared_ptr<ArrayInstance>,
    std::shared_ptr<Closure>,
    std::shared_ptr<Upvalue>,
    std::shared_ptr<ClassInstance>,
    std::shared_ptr<ClassDefinition>
>;

struct Upvalue {
    IRValue* location; 
    IRValue closed;
    bool is_closed = false;

    explicit Upvalue(IRValue* loc) : location(loc) {}
};

struct Closure {
    size_t ip;
    std::vector<std::shared_ptr<Upvalue>> upvalues;

    explicit Closure(size_t ip) : ip(ip) {}
    void clear() { upvalues.clear(); }
};

struct StructInstance : std::enable_shared_from_this<StructInstance> {
    std::unordered_map<std::string, IRValue> fields;
    void clear() { fields.clear(); }
};

struct ArrayInstance : std::enable_shared_from_this<ArrayInstance> {
    std::vector<IRValue> elements;
    void clear() { elements.clear(); }
};

struct ClassDefinition {
    std::string name;
    std::string superclass;
    std::unordered_map<std::string, IRValue> fields;
    std::unordered_map<std::string, size_t> methods; // name -> ip
    
    ClassDefinition(std::string n, std::string s = "") : name(std::move(n)), superclass(std::move(s)) {}
};

struct ClassInstance : std::enable_shared_from_this<ClassInstance> {
    std::shared_ptr<ClassDefinition> definition;
    std::unordered_map<std::string, IRValue> fields;
    
    explicit ClassInstance(std::shared_ptr<ClassDefinition> def) : definition(std::move(def)) {}
    void clear() { fields.clear(); definition.reset(); }
};

struct Instruction {
    OpCode opcode;
    IRValue operand;
    uint32_t line = 0;
    uint32_t column = 0;
    
    Instruction(OpCode op) : opcode(op), operand(std::monostate{}) {}
    Instruction(OpCode op, IRValue val) : opcode(op), operand(std::move(val)) {}
    Instruction(OpCode op, IRValue val, uint32_t l, uint32_t c) 
        : opcode(op), operand(std::move(val)), line(l), column(c) {}
};

} // namespace vyronix
