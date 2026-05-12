#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "AST.hpp"
#include "IR.hpp"

namespace vyronix {

class IRGenerator : public ASTVisitor {
public:
    [[nodiscard]] std::vector<Instruction> generate(const std::vector<std::unique_ptr<Stmt>>& statements);

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
    std::vector<Instruction> code_;
    std::unordered_map<std::string, size_t> function_labels_;
    std::vector<ClosureExpr*> closure_stack_;

    void collectFunctions(const std::vector<std::unique_ptr<Stmt>>& statements);
    [[nodiscard]] int emitJump(OpCode op);
    void patchJump(int idx) noexcept;
    void emit(OpCode op, IRValue val = std::monostate{});
};

} // namespace vyronix