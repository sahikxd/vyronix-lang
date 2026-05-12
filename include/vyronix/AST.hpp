#pragma once

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "Token.hpp"

namespace vyronix {

// Forward declarations
class Expr;
class Stmt;
class BinaryExpr;
class UnaryExpr;
class LiteralExpr;
class GroupingExpr;
class VariableExpr;
class AssignExpr;
class CallExpr;
class LogicalExpr;
class StructInstanceExpr;
class GetFieldExpr;
class SetFieldExpr;
class ArrayLiteralExpr;
class IndexExpr;
class SliceExpr;
class SetIndexExpr;
class TupleExpr;
class ClosureExpr;
class InterpolatedStringExpr;
class InvalidExpr;

class ExpressionStmt;
class VarStmt;
class ConstStmt;
class BlockStmt;
class IfStmt;
class LoopStmt;
class ForStmt;
class MatchStmt;
class TryStmt;
class ThrowStmt;
class FunctionStmt;
class ProcStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class StructStmt;
class EnumStmt;
class ImportStmt;
class InvalidStmt;

struct Type {
    enum class Kind { BASE, ARRAY, CUSTOM, TUPLE, FUNCTION, GENERIC };
    Kind kind;
    std::string name; // "i8", "u32", "str", "any", "tuple", or custom identifier
    std::unique_ptr<Type> element_type; // For arrays
    std::vector<std::unique_ptr<Type>> tuple_types; // For tuples
    std::vector<std::unique_ptr<Type>> type_args; // For generic types like List<i64>

    Type(Kind k, std::string n = "") : kind(k), name(std::move(n)) {}
    Type(std::unique_ptr<Type> elem) : kind(Kind::ARRAY), name("array"), element_type(std::move(elem)) {}
    Type(std::vector<std::unique_ptr<Type>> types) : kind(Kind::TUPLE), name("tuple"), tuple_types(std::move(types)) {}
    
    std::string getName() const {
        if (kind == Kind::ARRAY && element_type) return "array<" + element_type->getName() + ">";
        if (kind == Kind::TUPLE) {
            std::string s = "(";
            for (size_t i = 0; i < tuple_types.size(); ++i) {
                s += tuple_types[i]->getName();
                if (i < tuple_types.size() - 1) s += ", ";
            }
            s += ")";
            return s;
        }
        return name;
    }

    std::unique_ptr<Type> clone() const {
        auto t = std::make_unique<Type>(kind, name);
        if (element_type) t->element_type = element_type->clone();
        for (const auto& tt : tuple_types) {
            if (tt) t->tuple_types.push_back(tt->clone());
        }
        for (const auto& ta : type_args) {
            if (ta) t->type_args.push_back(ta->clone());
        }
        return t;
    }
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visitBinaryExpr(BinaryExpr& expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr& expr) = 0;
    virtual void visitLiteralExpr(LiteralExpr& expr) = 0;
    virtual void visitGroupingExpr(GroupingExpr& expr) = 0;
    virtual void visitVariableExpr(VariableExpr& expr) = 0;
    virtual void visitAssignExpr(AssignExpr& expr) = 0;
    virtual void visitCallExpr(CallExpr& expr) = 0;
    virtual void visitLogicalExpr(LogicalExpr& expr) = 0;
    virtual void visitStructInstanceExpr(StructInstanceExpr& expr) = 0;
    virtual void visitGetFieldExpr(GetFieldExpr& expr) = 0;
    virtual void visitSetFieldExpr(SetFieldExpr& expr) = 0;
    virtual void visitArrayLiteralExpr(ArrayLiteralExpr& expr) = 0;
    virtual void visitIndexExpr(IndexExpr& expr) = 0;
    virtual void visitSliceExpr(SliceExpr& expr) = 0;
    virtual void visitSetIndexExpr(SetIndexExpr& expr) = 0;
    virtual void visitTupleExpr(TupleExpr& expr) = 0;
    virtual void visitClosureExpr(ClosureExpr& expr) = 0;
    virtual void visitInterpolatedStringExpr(InterpolatedStringExpr& expr) = 0;
    virtual void visitInvalidExpr(InvalidExpr& expr) = 0;

    virtual void visitExpressionStmt(ExpressionStmt& stmt) = 0;
    virtual void visitVarStmt(VarStmt& stmt) = 0;
    virtual void visitConstStmt(ConstStmt& stmt) = 0;
    virtual void visitBlockStmt(BlockStmt& stmt) = 0;
    virtual void visitIfStmt(IfStmt& stmt) = 0;
    virtual void visitLoopStmt(LoopStmt& stmt) = 0;
    virtual void visitForStmt(ForStmt& stmt) = 0;
    virtual void visitMatchStmt(MatchStmt& stmt) = 0;
    virtual void visitTryStmt(TryStmt& stmt) = 0;
    virtual void visitThrowStmt(ThrowStmt& stmt) = 0;
    virtual void visitFunctionStmt(FunctionStmt& stmt) = 0;
    virtual void visitProcStmt(ProcStmt& stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt& stmt) = 0;
    virtual void visitBreakStmt(BreakStmt& stmt) = 0;
    virtual void visitContinueStmt(ContinueStmt& stmt) = 0;
    virtual void visitStructStmt(StructStmt& stmt) = 0;
    virtual void visitEnumStmt(EnumStmt& stmt) = 0;
    virtual void visitImportStmt(ImportStmt& stmt) = 0;
    virtual void visitInvalidStmt(InvalidStmt& stmt) = 0;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

class Expr : public ASTNode {
public:
    virtual std::unique_ptr<Expr> clone() const = 0;
};
class Stmt : public ASTNode {
public:
    virtual std::unique_ptr<Stmt> clone() const = 0;
};

struct Parameter {
    Token name;
    std::unique_ptr<Type> type;
    
    Parameter clone() const {
        return {name, type->clone()};
    }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) 
        : statements(std::move(statements)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitBlockStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        std::vector<std::unique_ptr<Stmt>> stmts;
        for (const auto& s : statements) if (s) stmts.push_back(s->clone());
        return std::make_unique<BlockStmt>(std::move(stmts));
    }
    std::unique_ptr<BlockStmt> cloneBlock() const {
        std::vector<std::unique_ptr<Stmt>> stmts;
        for (const auto& s : statements) if (s) stmts.push_back(s->clone());
        return std::make_unique<BlockStmt>(std::move(stmts));
    }
};

// Expressions
class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitBinaryExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<BinaryExpr>(left ? left->clone() : nullptr, op, right ? right->clone() : nullptr);
    }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op(std::move(op)), right(std::move(right)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitUnaryExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<UnaryExpr>(op, right ? right->clone() : nullptr);
    }
};

class LiteralExpr : public Expr {
public:
    TokenValue value;
    LiteralExpr(TokenValue value) : value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitLiteralExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<LiteralExpr>(value);
    }
};

class GroupingExpr : public Expr {
public:
    std::unique_ptr<Expr> expression;
    GroupingExpr(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitGroupingExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<GroupingExpr>(expression ? expression->clone() : nullptr);
    }
};

class VariableExpr : public Expr {
public:
    Token name;
    bool is_capture = false;
    VariableExpr(Token name) : name(std::move(name)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitVariableExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        auto v = std::make_unique<VariableExpr>(name);
        v->is_capture = is_capture;
        return v;
    }
};

class AssignExpr : public Expr {
public:
    Token name;
    std::unique_ptr<Expr> value;
    bool is_capture = false;
    AssignExpr(Token name, std::unique_ptr<Expr> value)
        : name(std::move(name)), value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitAssignExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        auto a = std::make_unique<AssignExpr>(name, value->clone());
        a->is_capture = is_capture;
        return a;
    }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    Token paren;
    std::vector<std::unique_ptr<Expr>> arguments;
    std::vector<std::unique_ptr<Type>> type_args; // Added
    CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments, std::vector<std::unique_ptr<Type>> type_args = {})
        : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)), type_args(std::move(type_args)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitCallExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> args;
        for (const auto& a : arguments) args.push_back(a ? a->clone() : nullptr);
        std::vector<std::unique_ptr<Type>> t_args;
        for (const auto& t : type_args) t_args.push_back(t ? t->clone() : nullptr);
        return std::make_unique<CallExpr>(callee ? callee->clone() : nullptr, paren, std::move(args), std::move(t_args));
    }
};

class LogicalExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    LogicalExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitLogicalExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<LogicalExpr>(left ? left->clone() : nullptr, op, right ? right->clone() : nullptr);
    }
};

class StructInstanceExpr : public Expr {
public:
    Token struct_name;
    std::vector<std::pair<Token, std::unique_ptr<Expr>>> initializers;
    StructInstanceExpr(Token struct_name, std::vector<std::pair<Token, std::unique_ptr<Expr>>> initializers)
        : struct_name(std::move(struct_name)), initializers(std::move(initializers)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitStructInstanceExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::pair<Token, std::unique_ptr<Expr>>> inits;
        for (const auto& i : initializers) inits.push_back({i.first, i.second ? i.second->clone() : nullptr});
        return std::make_unique<StructInstanceExpr>(struct_name, std::move(inits));
    }
};

class ArrayLiteralExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitArrayLiteralExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> elms;
        for (const auto& e : elements) elms.push_back(e ? e->clone() : nullptr);
        return std::make_unique<ArrayLiteralExpr>(std::move(elms));
    }
};

class GetFieldExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token name;
    GetFieldExpr(std::unique_ptr<Expr> object, Token name)
        : object(std::move(object)), name(std::move(name)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitGetFieldExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<GetFieldExpr>(object ? object->clone() : nullptr, name);
    }
};

class SetFieldExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token name;
    std::unique_ptr<Expr> value;
    SetFieldExpr(std::unique_ptr<Expr> object, Token name, std::unique_ptr<Expr> value)
        : object(std::move(object)), name(std::move(name)), value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitSetFieldExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<SetFieldExpr>(object ? object->clone() : nullptr, name, value ? value->clone() : nullptr);
    }
};

class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token bracket;
    std::unique_ptr<Expr> index;
    IndexExpr(std::unique_ptr<Expr> object, Token bracket, std::unique_ptr<Expr> index)
        : object(std::move(object)), bracket(std::move(bracket)), index(std::move(index)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitIndexExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<IndexExpr>(object ? object->clone() : nullptr, bracket, index ? index->clone() : nullptr);
    }
};

class SliceExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    Token bracket;
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    SliceExpr(std::unique_ptr<Expr> object, Token bracket, std::unique_ptr<Expr> start, std::unique_ptr<Expr> end)
        : object(std::move(object)), bracket(std::move(bracket)), start(std::move(start)), end(std::move(end)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitSliceExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<SliceExpr>(object->clone(), bracket, 
            start ? start->clone() : nullptr, 
            end ? end->clone() : nullptr);
    }
};

class SetIndexExpr : public Expr {
public:
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;
    std::unique_ptr<Expr> value;
    SetIndexExpr(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index, std::unique_ptr<Expr> value)
        : object(std::move(object)), index(std::move(index)), value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitSetIndexExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<SetIndexExpr>(object ? object->clone() : nullptr, index ? index->clone() : nullptr, value ? value->clone() : nullptr);
    }
};

class TupleExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    TupleExpr(std::vector<std::unique_ptr<Expr>> elements) : elements(std::move(elements)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitTupleExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> cloned;
        for (const auto& e : elements) cloned.push_back(e ? e->clone() : nullptr);
        return std::make_unique<TupleExpr>(std::move(cloned));
    }
};

class ClosureExpr : public Expr {
public:
    std::vector<Parameter> params;
    std::unique_ptr<Type> return_type;
    std::unique_ptr<BlockStmt> body;
    std::vector<std::string> captured_vars;

    ClosureExpr(std::vector<Parameter> params, std::unique_ptr<Type> return_type, std::unique_ptr<BlockStmt> body)
        : params(std::move(params)), return_type(std::move(return_type)), body(std::move(body)) {}
    
    void accept(ASTVisitor& visitor) override { visitor.visitClosureExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        std::vector<Parameter> ps;
        for (const auto& p : params) ps.push_back(p.clone());
        return std::make_unique<ClosureExpr>(std::move(ps), 
            return_type ? return_type->clone() : nullptr, 
            body ? body->cloneBlock() : nullptr);
    }
};

class InterpolatedStringExpr : public Expr {
public:
    std::vector<std::variant<std::string, std::unique_ptr<Expr>>> parts;
    InterpolatedStringExpr(std::vector<std::variant<std::string, std::unique_ptr<Expr>>> parts)
        : parts(std::move(parts)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitInterpolatedStringExpr(*this); }
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::variant<std::string, std::unique_ptr<Expr>>> ps;
        for (const auto& p : parts) {
            if (std::holds_alternative<std::string>(p)) ps.push_back(std::get<std::string>(p));
            else ps.push_back(std::get<std::unique_ptr<Expr>>(p)->clone());
        }
        return std::make_unique<InterpolatedStringExpr>(std::move(ps));
    }
};

class InvalidExpr : public Expr {
public:
    std::string reason;
    InvalidExpr(std::string reason) : reason(std::move(reason)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitInvalidExpr(*this); }
    std::unique_ptr<Expr> clone() const override { return std::make_unique<InvalidExpr>(reason); }
};

// Statements
class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitExpressionStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ExpressionStmt>(expression ? expression->clone() : nullptr);
    }
};

class VarStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Type> type;
    std::unique_ptr<Expr> initializer;
    VarStmt(Token name, std::unique_ptr<Type> type, std::unique_ptr<Expr> initializer)
        : name(std::move(name)), type(std::move(type)), initializer(std::move(initializer)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitVarStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<VarStmt>(name, type ? type->clone() : nullptr, initializer ? initializer->clone() : nullptr);
    }
};

class ConstStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Type> type;
    std::unique_ptr<Expr> initializer;
    ConstStmt(Token name, std::unique_ptr<Type> type, std::unique_ptr<Expr> initializer)
        : name(std::move(name)), type(std::move(type)), initializer(std::move(initializer)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitConstStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ConstStmt>(name, type ? type->clone() : nullptr, initializer ? initializer->clone() : nullptr);
    }
};

class IfStmt : public Stmt {
public:
    struct ElifBranch {
        std::unique_ptr<Expr> condition;
        std::unique_ptr<BlockStmt> body;
    };
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> thenBranch;
    std::vector<ElifBranch> elifBranches;
    std::unique_ptr<BlockStmt> elseBranch;
    
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> thenBranch, 
           std::vector<ElifBranch> elifBranches, std::unique_ptr<BlockStmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), 
          elifBranches(std::move(elifBranches)), elseBranch(std::move(elseBranch)) {}
          
    void accept(ASTVisitor& visitor) override { visitor.visitIfStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        std::vector<ElifBranch> elifs;
        for (const auto& e : elifBranches) {
            elifs.push_back({e.condition ? e.condition->clone() : nullptr, e.body ? e.body->cloneBlock() : nullptr});
        }
        return std::make_unique<IfStmt>(condition ? condition->clone() : nullptr, 
            thenBranch ? thenBranch->cloneBlock() : nullptr, 
            std::move(elifs), 
            elseBranch ? elseBranch->cloneBlock() : nullptr);
    }
};

class LoopStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<BlockStmt> body;
    LoopStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitLoopStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<LoopStmt>(condition ? condition->clone() : nullptr, 
            body ? body->cloneBlock() : nullptr);
    }
};

class ForStmt : public Stmt {
public:
    Token iterator;
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    std::unique_ptr<Expr> step;
    std::unique_ptr<BlockStmt> body;
    
    ForStmt(Token iterator, std::unique_ptr<Expr> start, std::unique_ptr<Expr> end, 
            std::unique_ptr<Expr> step, std::unique_ptr<BlockStmt> body)
        : iterator(std::move(iterator)), start(std::move(start)), end(std::move(end)), 
          step(std::move(step)), body(std::move(body)) {}
          
    void accept(ASTVisitor& visitor) override { visitor.visitForStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ForStmt>(iterator, start ? start->clone() : nullptr, end ? end->clone() : nullptr, 
            step ? step->clone() : nullptr, 
            body ? body->cloneBlock() : nullptr);
    }
};

class FunctionStmt : public Stmt {
public:
    Token name;
    std::vector<std::string> type_params;
    std::vector<Parameter> params;
    std::unique_ptr<Type> return_type;
    std::unique_ptr<BlockStmt> body;
    FunctionStmt(Token name, std::vector<Parameter> params, std::unique_ptr<Type> return_type, std::unique_ptr<BlockStmt> body, std::vector<std::string> type_params = {})
        : name(std::move(name)), type_params(std::move(type_params)), params(std::move(params)), return_type(std::move(return_type)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitFunctionStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        std::vector<Parameter> ps;
        for (const auto& p : params) ps.push_back(p.clone());
        return std::make_unique<FunctionStmt>(name, std::move(ps), 
            return_type ? return_type->clone() : nullptr, 
            body ? body->cloneBlock() : nullptr,
            type_params);
    }
};

class ProcStmt : public Stmt {
public:
    Token name;
    std::vector<std::string> type_params;
    std::vector<Parameter> params;
    std::unique_ptr<BlockStmt> body;
    ProcStmt(Token name, std::vector<Parameter> params, std::unique_ptr<BlockStmt> body, std::vector<std::string> type_params = {})
        : name(std::move(name)), type_params(std::move(type_params)), params(std::move(params)), body(std::move(body)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitProcStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        std::vector<Parameter> ps;
        for (const auto& p : params) ps.push_back(p.clone());
        return std::make_unique<ProcStmt>(name, std::move(ps), 
            body ? body->cloneBlock() : nullptr,
            type_params);
    }
};

class MatchCase {
public:
    std::unique_ptr<Expr> literal; // literal or nullptr for default
    std::unique_ptr<Stmt> statement;
};

class MatchStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    std::vector<MatchCase> cases;
    MatchStmt(std::unique_ptr<Expr> expression, std::vector<MatchCase> cases)
        : expression(std::move(expression)), cases(std::move(cases)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitMatchStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        std::vector<MatchCase> cs;
        for (const auto& c : cases) {
            cs.push_back({c.literal ? c.literal->clone() : nullptr, c.statement ? c.statement->clone() : nullptr});
        }
        return std::make_unique<MatchStmt>(expression ? expression->clone() : nullptr, std::move(cs));
    }
};

class TryStmt : public Stmt {
public:
    std::unique_ptr<BlockStmt> tryBlock;
    Token catchVar;
    std::unique_ptr<BlockStmt> catchBlock;
    TryStmt(std::unique_ptr<BlockStmt> tryBlock, Token catchVar, std::unique_ptr<BlockStmt> catchBlock)
        : tryBlock(std::move(tryBlock)), catchVar(std::move(catchVar)), catchBlock(std::move(catchBlock)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitTryStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<TryStmt>(
            tryBlock ? tryBlock->cloneBlock() : nullptr,
            catchVar,
            catchBlock ? catchBlock->cloneBlock() : nullptr
        );
    }
};

class ThrowStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> expression;
    ThrowStmt(Token keyword, std::unique_ptr<Expr> expression)
        : keyword(std::move(keyword)), expression(std::move(expression)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitThrowStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ThrowStmt>(keyword, expression ? expression->clone() : nullptr);
    }
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;
    ReturnStmt(Token keyword, std::unique_ptr<Expr> value)
        : keyword(std::move(keyword)), value(std::move(value)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitReturnStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ReturnStmt>(keyword, value ? value->clone() : nullptr);
    }
};

class BreakStmt : public Stmt {
public:
    Token keyword;
    BreakStmt(Token keyword) : keyword(std::move(keyword)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitBreakStmt(*this); }
    std::unique_ptr<Stmt> clone() const override { return std::make_unique<BreakStmt>(keyword); }
};

class ContinueStmt : public Stmt {
public:
    Token keyword;
    ContinueStmt(Token keyword) : keyword(std::move(keyword)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitContinueStmt(*this); }
    std::unique_ptr<Stmt> clone() const override { return std::make_unique<ContinueStmt>(keyword); }
};

class StructStmt : public Stmt {
public:
    Token name;
    std::vector<std::string> type_params;
    std::vector<Parameter> fields;
    StructStmt(Token name, std::vector<Parameter> fields, std::vector<std::string> type_params = {})
        : name(std::move(name)), type_params(std::move(type_params)), fields(std::move(fields)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitStructStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        std::vector<Parameter> fs;
        for (const auto& f : fields) fs.push_back(f.clone());
        return std::make_unique<StructStmt>(name, std::move(fs), type_params);
    }
};

class EnumStmt : public Stmt {
public:
    Token name;
    std::vector<Token> members;
    EnumStmt(Token name, std::vector<Token> members)
        : name(std::move(name)), members(std::move(members)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitEnumStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<EnumStmt>(name, members);
    }
};

class ImportStmt : public Stmt {
public:
    enum class Kind { STRING, MODULE };
    Kind kind;
    std::string path_or_module;
    bool is_export;
    
    ImportStmt(Kind k, std::string p, bool exp = false) 
        : kind(k), path_or_module(std::move(p)), is_export(exp) {}
        
    void accept(ASTVisitor& visitor) override { visitor.visitImportStmt(*this); }
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ImportStmt>(kind, path_or_module, is_export);
    }
};

class InvalidStmt : public Stmt {
public:
    std::string reason;
    InvalidStmt(std::string reason) : reason(std::move(reason)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitInvalidStmt(*this); }
    std::unique_ptr<Stmt> clone() const override { return std::make_unique<InvalidStmt>(reason); }
};

} // namespace vyronix