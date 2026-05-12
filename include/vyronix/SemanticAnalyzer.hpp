#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <memory>
#include <set>
#include "AST.hpp"

namespace vyronix {

struct SymbolInfo {
    std::unique_ptr<Type> type;
    bool is_const;
    bool is_function;
    bool is_proc;
    std::vector<std::string> type_params;
    std::vector<std::unique_ptr<Type>> param_types;
    std::unique_ptr<Type> return_type;
    bool is_enum = false;
    std::string enum_name;
    bool is_struct = false;
    std::unordered_map<std::string, std::unique_ptr<Type>> field_types;

    SymbolInfo() = default;
    SymbolInfo(SymbolInfo&&) = default;
    SymbolInfo& operator=(SymbolInfo&&) = default;

    SymbolInfo(const SymbolInfo& other) {
        if (other.type) type = other.type->clone();
        is_const = other.is_const;
        is_function = other.is_function;
        is_proc = other.is_proc;
        type_params = other.type_params;
        for (const auto& pt : other.param_types) {
            if (pt) param_types.push_back(pt->clone());
        }
        if (other.return_type) return_type = other.return_type->clone();
        is_enum = other.is_enum;
        enum_name = other.enum_name;
        is_struct = other.is_struct;
        for (const auto& [name, type_ptr] : other.field_types) {
            if (type_ptr) field_types[name] = type_ptr->clone();
        }
    }

    SymbolInfo& operator=(const SymbolInfo& other) {
        if (this == &other) return *this;
        if (other.type) type = other.type->clone();
        else type.reset();
        is_const = other.is_const;
        is_function = other.is_function;
        is_proc = other.is_proc;
        type_params = other.type_params;
        param_types.clear();
        for (const auto& pt : other.param_types) {
            if (pt) param_types.push_back(pt->clone());
        }
        if (other.return_type) return_type = other.return_type->clone();
        else return_type.reset();
        is_enum = other.is_enum;
        enum_name = other.enum_name;
        is_struct = other.is_struct;
        field_types.clear();
        for (const auto& [name, type_ptr] : other.field_types) {
            if (type_ptr) field_types[name] = type_ptr->clone();
        }
        return *this;
    }
};

class SemanticAnalyzer : public ASTVisitor {
public:
    void analyze(const std::vector<std::unique_ptr<Stmt>>& statements, std::string filename = "unknown");
    
    void visitBinaryExpr(BinaryExpr& expr) override;
    void visitUnaryExpr(UnaryExpr& expr) override;
    void visitLiteralExpr(LiteralExpr& expr) override;
    void visitGroupingExpr(GroupingExpr& expr) override;
    void visitVariableExpr(VariableExpr& expr) override;
    void visitAssignExpr(AssignExpr& expr) override;
    void visitCallExpr(CallExpr& expr) override;
    void visitLogicalExpr(LogicalExpr& expr) override;
    void visitStructInstanceExpr(StructInstanceExpr& expr) override;
    void visitGetFieldExpr(GetFieldExpr& expr) override;
    void visitSetFieldExpr(SetFieldExpr& expr) override;
    void visitArrayLiteralExpr(ArrayLiteralExpr& expr) override;
    void visitIndexExpr(IndexExpr& expr) override;
    void visitSliceExpr(SliceExpr& expr) override;
    void visitSetIndexExpr(SetIndexExpr& expr) override;
    void visitTupleExpr(TupleExpr& expr) override;
    void visitClosureExpr(ClosureExpr& expr) override;
    void visitInterpolatedStringExpr(InterpolatedStringExpr& expr) override;
    void visitInvalidExpr(InvalidExpr& expr) override;

    void visitExpressionStmt(ExpressionStmt& stmt) override;
    void visitVarStmt(VarStmt& stmt) override;
    void visitConstStmt(ConstStmt& stmt) override;
    void visitBlockStmt(BlockStmt& stmt) override;
    void visitIfStmt(IfStmt& stmt) override;
    void visitLoopStmt(LoopStmt& stmt) override;
    void visitForStmt(ForStmt& stmt) override;
    void visitMatchStmt(MatchStmt& stmt) override;
    void visitTryStmt(TryStmt& stmt) override;
    void visitThrowStmt(ThrowStmt& stmt) override;
    void visitFunctionStmt(FunctionStmt& stmt) override;
    void visitProcStmt(ProcStmt& stmt) override;
    void visitReturnStmt(ReturnStmt& stmt) override;
    void visitBreakStmt(BreakStmt& stmt) override;
    void visitContinueStmt(ContinueStmt& stmt) override;
    void visitStructStmt(StructStmt& stmt) override;
    void visitEnumStmt(EnumStmt& stmt) override;
    void visitImportStmt(ImportStmt& stmt) override;
    void visitInvalidStmt(InvalidStmt& stmt) override;

private:
    struct StructInfo {
        std::unordered_map<std::string, std::unique_ptr<Type>> fields;
    };

    struct EnumInfo {
        std::unordered_map<std::string, int> members;
    };

    std::unordered_map<std::string, StructInfo> structs_;
    std::unordered_map<std::string, EnumInfo> enums_;
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes_;
    std::string filename_;
    std::unique_ptr<Type> current_expr_type_;
    std::unique_ptr<Type> current_function_return_type_;
    std::vector<ClosureExpr*> closure_stack_;
    std::vector<size_t> closure_scope_depths_;
    std::vector<std::set<std::string>> type_param_stack_;

    void beginScope() noexcept;
    void endScope() noexcept;
    void declare(const Token& name, SymbolInfo info);
    [[nodiscard]] std::optional<SymbolInfo> resolve(const std::string& name) noexcept;
    [[nodiscard]] std::pair<std::optional<SymbolInfo>, int> resolveWithDepth(const std::string& name) noexcept;
    
    [[nodiscard]] std::unique_ptr<Type> substituteTypes(const Type& type, const std::unordered_map<std::string, const Type*>& substitutions);

    void error(const Token& token, const std::string& message);
    [[nodiscard]] bool checkTypes(const Type& left, const Type& right) noexcept;
    [[nodiscard]] std::unique_ptr<Type> inferType(Expr& expr);
};

} // namespace vyronix