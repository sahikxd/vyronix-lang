#include "vyronix/IRGenerator.hpp"
#include "vyronix/NativeRegistry.hpp"
#include <iostream>
#include <algorithm>

namespace vyronix {

std::vector<Instruction> IRGenerator::generate(const std::vector<std::unique_ptr<Stmt>>& statements) {
    int skip_functions = emitJump(OpCode::JUMP);
    collectFunctions(statements);
    patchJump(skip_functions);

    for (const auto& stmt : statements) {
        if (stmt && !dynamic_cast<FunctionStmt*>(stmt.get()) && !dynamic_cast<ProcStmt*>(stmt.get())) {
            stmt->accept(*this);
        }
    }
    emit(OpCode::HALT);
    return code_;
}

void IRGenerator::collectFunctions(const std::vector<std::unique_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        if (auto func = dynamic_cast<FunctionStmt*>(stmt.get())) {
            function_labels_[func->name.lexeme] = code_.size();
            // Function body: arguments are pushed by caller, pop them here
            for (auto it = func->params.rbegin(); it != func->params.rend(); ++it) {
                emit(OpCode::STORE_VAR, it->name.lexeme);
                emit(OpCode::POP);
            }
            if (func->body) func->body->accept(*this);
            emit(OpCode::PUSH_CONST, std::monostate{});
            emit(OpCode::RETURN);
        } else if (auto proc = dynamic_cast<ProcStmt*>(stmt.get())) {
            function_labels_[proc->name.lexeme] = code_.size();
            // Proc body: arguments are pushed by caller, pop them here
            for (auto it = proc->params.rbegin(); it != proc->params.rend(); ++it) {
                emit(OpCode::STORE_VAR, it->name.lexeme);
                emit(OpCode::POP);
            }
            if (proc->body) proc->body->accept(*this);
            emit(OpCode::RETURN);
        } else if (auto block = dynamic_cast<BlockStmt*>(stmt.get())) {
            collectFunctions(block->statements);
        }
    }
}

void IRGenerator::visitBinaryExpr(BinaryExpr& expr) {
    if (expr.left) expr.left->accept(*this);
    if (expr.right) expr.right->accept(*this);
    switch (expr.op.type) {
        case TokenType::PLUS: emit(OpCode::ADD); break;
        case TokenType::MINUS: emit(OpCode::SUB); break;
        case TokenType::STAR: emit(OpCode::MUL); break;
        case TokenType::SLASH: emit(OpCode::DIV); break;
        case TokenType::PERCENT: emit(OpCode::MOD); break;
        case TokenType::EQ_EQ: emit(OpCode::EQ); break;
        case TokenType::BANG_EQ: emit(OpCode::NE); break;
        case TokenType::LT: emit(OpCode::LT); break;
        case TokenType::LT_EQ: emit(OpCode::LE); break;
        case TokenType::GT: emit(OpCode::GT); break;
        case TokenType::GT_EQ: emit(OpCode::GE); break;
        default: break;
    }
}

void IRGenerator::visitUnaryExpr(UnaryExpr& expr) {
    if (expr.right) expr.right->accept(*this);
    if (expr.op.type == TokenType::MINUS) {
        emit(OpCode::PUSH_CONST, (int64_t)-1);
        emit(OpCode::MUL);
    } else if (expr.op.type == TokenType::NOT) {
        emit(OpCode::NOT);
    }
}

void IRGenerator::visitLiteralExpr(LiteralExpr& expr) {
    if (std::holds_alternative<int64_t>(expr.value)) emit(OpCode::PUSH_CONST, std::get<int64_t>(expr.value));
    else if (std::holds_alternative<double>(expr.value)) emit(OpCode::PUSH_CONST, std::get<double>(expr.value));
    else if (std::holds_alternative<std::string>(expr.value)) emit(OpCode::PUSH_CONST, std::get<std::string>(expr.value));
    else if (std::holds_alternative<bool>(expr.value)) emit(OpCode::PUSH_CONST, std::get<bool>(expr.value));
    else emit(OpCode::PUSH_CONST, std::monostate{});
}

void IRGenerator::visitGroupingExpr(GroupingExpr& expr) {
    if (expr.expression) expr.expression->accept(*this);
}

void IRGenerator::visitVariableExpr(VariableExpr& expr) {
    if (expr.is_capture && !closure_stack_.empty()) {
        const auto& captured = closure_stack_.back()->captured_vars;
        auto it = std::find(captured.begin(), captured.end(), expr.name.lexeme);
        if (it != captured.end()) {
            emit(OpCode::LOAD_UPVALUE, (int64_t)std::distance(captured.begin(), it));
            return;
        }
    }
    emit(OpCode::LOAD_VAR, expr.name.lexeme);
}

void IRGenerator::visitAssignExpr(AssignExpr& expr) {
    if (expr.value) expr.value->accept(*this);
    if (expr.is_capture && !closure_stack_.empty()) {
        const auto& captured = closure_stack_.back()->captured_vars;
        auto it = std::find(captured.begin(), captured.end(), expr.name.lexeme);
        if (it != captured.end()) {
            emit(OpCode::STORE_UPVALUE, (int64_t)std::distance(captured.begin(), it));
            return;
        }
    }
    emit(OpCode::STORE_VAR, expr.name.lexeme);
}

void IRGenerator::visitClosureExpr(ClosureExpr& expr) {
    int skip_body = emitJump(OpCode::JUMP);
    size_t body_ip = code_.size();
    
    closure_stack_.push_back(&expr);
    
    // Closure params: same as functions
    for (auto it = expr.params.rbegin(); it != expr.params.rend(); ++it) {
        emit(OpCode::STORE_VAR, it->name.lexeme);
        emit(OpCode::POP);
    }
    
    if (expr.body) expr.body->accept(*this);
    
    // Implicit return null
    emit(OpCode::PUSH_CONST, std::monostate{});
    emit(OpCode::RETURN);
    
    closure_stack_.pop_back();
    patchJump(skip_body);
    
    // Now emit MAKE_CLOSURE
    emit(OpCode::PUSH_CONST, (int64_t)body_ip);
    
    // First, push capture descriptions
    // Each capture: [is_local (bool), index_or_name (int/str)]
    for (const auto& var : expr.captured_vars) {
        bool is_local = true;
        int64_t upvalue_idx = -1;
        
        // If we are already inside a closure, check if 'var' is an upvalue there
        if (!closure_stack_.empty()) {
            const auto& parent_captured = closure_stack_.back()->captured_vars;
            auto it = std::find(parent_captured.begin(), parent_captured.end(), var);
            if (it != parent_captured.end()) {
                is_local = false;
                upvalue_idx = std::distance(parent_captured.begin(), it);
            }
        }
        
        emit(OpCode::PUSH_CONST, is_local);
        if (is_local) {
            emit(OpCode::PUSH_CONST, var);
        } else {
            emit(OpCode::PUSH_CONST, upvalue_idx);
        }
    }
    
    emit(OpCode::MAKE_CLOSURE, (int64_t)expr.captured_vars.size());
    // VM will pop 2 * captured_vars.size() operands to build the closure
}

void IRGenerator::visitCallExpr(CallExpr& expr) {
    for (auto& arg : expr.arguments) {
        if (arg) arg->accept(*this);
    }
    
    if (auto* v = dynamic_cast<VariableExpr*>(expr.callee.get())) {
        auto it = function_labels_.find(v->name.lexeme);
        if (it != function_labels_.end()) {
            emit(OpCode::PUSH_CONST, Label{(int64_t)it->second});
        } else {
            // Check if it's a native function
            if (NativeRegistry::getInstance().exists(v->name.lexeme)) {
                emit(OpCode::PUSH_CONST, v->name.lexeme);
            } else {
                emit(OpCode::LOAD_VAR, v->name.lexeme);
            }
        }
    } else if (auto* g = dynamic_cast<GetFieldExpr*>(expr.callee.get())) {
        // Handle namespaced calls like io.print()
        if (auto* obj = dynamic_cast<VariableExpr*>(g->object.get())) {
            std::string ns = obj->name.lexeme;
            if (ns == "io" || ns == "fs" || ns == "str" || ns == "arr" || ns == "math" || ns == "sys" || ns == "weak") {
                std::string full_name = ns + "." + g->name.lexeme;
                if (NativeRegistry::getInstance().exists(full_name)) {
                    emit(OpCode::PUSH_CONST, full_name);
                } else {
                    expr.callee->accept(*this);
                }
            } else {
                expr.callee->accept(*this);
            }
        } else {
            expr.callee->accept(*this);
        }
    } else {
        expr.callee->accept(*this);
    }
    
    emit(OpCode::CALL, (int64_t)expr.arguments.size());
}

void IRGenerator::visitLogicalExpr(LogicalExpr& expr) {
    if (expr.left) expr.left->accept(*this);
    if (expr.right) expr.right->accept(*this);
    if (expr.op.type == TokenType::OR) emit(OpCode::OR);
    else emit(OpCode::AND);
}

void IRGenerator::visitStructInstanceExpr(StructInstanceExpr& expr) {
    for (const auto& init : expr.initializers) {
        if (init.second) init.second->accept(*this);
        emit(OpCode::PUSH_CONST, init.first.lexeme);
    }
    emit(OpCode::NEW_STRUCT, (int64_t)expr.initializers.size());
}

void IRGenerator::visitGetFieldExpr(GetFieldExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    emit(OpCode::GET_FIELD, expr.name.lexeme);
}

void IRGenerator::visitSetFieldExpr(SetFieldExpr& expr) {
    if (expr.object) expr.object->accept(*this); // Push object
    if (expr.value) expr.value->accept(*this);   // Push value
    emit(OpCode::SET_FIELD, expr.name.lexeme);
}

void IRGenerator::visitArrayLiteralExpr(ArrayLiteralExpr& expr) {
    for (const auto& e : expr.elements) if (e) e->accept(*this);
    emit(OpCode::NEW_ARRAY, (int64_t)expr.elements.size());
}

void IRGenerator::visitTupleExpr(TupleExpr& expr) {
    for (const auto& e : expr.elements) if (e) e->accept(*this);
    emit(OpCode::NEW_ARRAY, (int64_t)expr.elements.size());
}

void IRGenerator::visitIndexExpr(IndexExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    if (expr.index) expr.index->accept(*this);
    emit(OpCode::GET_INDEX);
}

void IRGenerator::visitSliceExpr(SliceExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    if (expr.start) expr.start->accept(*this); else emit(OpCode::PUSH_CONST, (int64_t)0);
    if (expr.end) expr.end->accept(*this); else emit(OpCode::PUSH_CONST, (int64_t)-1);
    emit(OpCode::GET_SLICE);
}

void IRGenerator::visitSetIndexExpr(SetIndexExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    if (expr.index) expr.index->accept(*this);
    if (expr.value) expr.value->accept(*this);
    emit(OpCode::SET_INDEX);
}

void IRGenerator::visitInterpolatedStringExpr(InterpolatedStringExpr& expr) {
    bool first = true;
    for (auto& part : expr.parts) {
        if (std::holds_alternative<std::string>(part)) {
            emit(OpCode::PUSH_CONST, std::get<std::string>(part));
        } else {
            std::get<std::unique_ptr<Expr>>(part)->accept(*this);
        }
        if (!first) emit(OpCode::ADD);
        first = false;
    }
}

void IRGenerator::visitInvalidExpr(InvalidExpr& expr) {
    (void)expr;
    emit(OpCode::NOP);
}

void IRGenerator::visitExpressionStmt(ExpressionStmt& stmt) {
    if (stmt.expression) {
        stmt.expression->accept(*this);
        emit(OpCode::POP);
    }
}

void IRGenerator::visitVarStmt(VarStmt& stmt) {
    if (stmt.initializer) stmt.initializer->accept(*this);
    else emit(OpCode::PUSH_CONST, std::monostate{});
    emit(OpCode::STORE_VAR, stmt.name.lexeme);
    emit(OpCode::POP);
}

void IRGenerator::visitConstStmt(ConstStmt& stmt) {
    if (stmt.initializer) stmt.initializer->accept(*this);
    emit(OpCode::STORE_VAR, stmt.name.lexeme);
    emit(OpCode::POP);
}

void IRGenerator::visitBlockStmt(BlockStmt& stmt) {
    for (const auto& s : stmt.statements) if (s) s->accept(*this);
}

void IRGenerator::visitIfStmt(IfStmt& stmt) {
    if (stmt.condition) stmt.condition->accept(*this);
    int next_jump = emitJump(OpCode::JUMP_IF_FALSE);
    if (stmt.thenBranch) stmt.thenBranch->accept(*this);
    std::vector<int> jumps;
    jumps.push_back(emitJump(OpCode::JUMP));
    patchJump(next_jump);

    for (auto& elif : stmt.elifBranches) {
        if (elif.condition) elif.condition->accept(*this);
        next_jump = emitJump(OpCode::JUMP_IF_FALSE);
        if (elif.body) elif.body->accept(*this);
        jumps.push_back(emitJump(OpCode::JUMP));
        patchJump(next_jump);
    }

    if (stmt.elseBranch) stmt.elseBranch->accept(*this);
    for (int j : jumps) patchJump(j);
}

void IRGenerator::visitLoopStmt(LoopStmt& stmt) {
    int start = code_.size();
    if (stmt.condition) stmt.condition->accept(*this);
    int end = emitJump(OpCode::JUMP_IF_FALSE);
    if (stmt.body) stmt.body->accept(*this);
    emit(OpCode::JUMP, (int64_t)start);
    patchJump(end);
}

void IRGenerator::visitForStmt(ForStmt& stmt) {
    if (stmt.start) stmt.start->accept(*this);
    emit(OpCode::STORE_VAR, stmt.iterator.lexeme);
    emit(OpCode::POP);

    int start_loop = code_.size();
    emit(OpCode::LOAD_VAR, stmt.iterator.lexeme);
    if (stmt.end) stmt.end->accept(*this);
    emit(OpCode::LE);
    int end_jump = emitJump(OpCode::JUMP_IF_FALSE);

    if (stmt.body) stmt.body->accept(*this);

    // Increment
    emit(OpCode::LOAD_VAR, stmt.iterator.lexeme);
    if (stmt.step) stmt.step->accept(*this); else emit(OpCode::PUSH_CONST, (int64_t)1);
    emit(OpCode::ADD);
    emit(OpCode::STORE_VAR, stmt.iterator.lexeme);
    emit(OpCode::POP);

    emit(OpCode::JUMP, Label{(int64_t)start_loop});
    patchJump(end_jump);
}

void IRGenerator::visitMatchStmt(MatchStmt& stmt) {
    if (stmt.expression) stmt.expression->accept(*this);
    std::vector<int> jumps;
    for (auto& c : stmt.cases) {
        if (c.literal) {
            emit(OpCode::DUP);
            c.literal->accept(*this);
            emit(OpCode::EQ);
            int skip = emitJump(OpCode::JUMP_IF_FALSE);
            emit(OpCode::POP); // Pop expression after match
            if (c.statement) c.statement->accept(*this);
            jumps.push_back(emitJump(OpCode::JUMP));
            patchJump(skip);
        } else {
            // Default case
            emit(OpCode::POP); // Pop expression
            if (c.statement) c.statement->accept(*this);
            jumps.push_back(emitJump(OpCode::JUMP));
            // Default case always matches, so we don't need to worry about fallthrough here
        }
    }
    // Fallthrough: pop the expression if no cases matched
    emit(OpCode::POP); 
    for (int j : jumps) patchJump(j);
}

void IRGenerator::visitTryStmt(TryStmt& stmt) {
    int catch_jump = emitJump(OpCode::TRY_BEGIN);
    if (stmt.tryBlock) stmt.tryBlock->accept(*this);
    emit(OpCode::TRY_END);
    int end_jump = emitJump(OpCode::JUMP);
    
    patchJump(catch_jump);
    // exception object is on stack
    emit(OpCode::STORE_VAR, stmt.catchVar.lexeme);
    emit(OpCode::POP);
    if (stmt.catchBlock) stmt.catchBlock->accept(*this);
    patchJump(end_jump);
}

void IRGenerator::visitThrowStmt(ThrowStmt& stmt) {
    if (stmt.expression) stmt.expression->accept(*this);
    emit(OpCode::THROW);
}

void IRGenerator::visitFunctionStmt(FunctionStmt& stmt) { (void)stmt; }

void IRGenerator::visitProcStmt(ProcStmt& stmt) { 
    (void)stmt;
    // Procedures return 'void' (monostate)
    emit(OpCode::PUSH_CONST, std::monostate{});
    emit(OpCode::RETURN);
}

void IRGenerator::visitReturnStmt(ReturnStmt& stmt) {
    if (stmt.value) stmt.value->accept(*this);
    else emit(OpCode::PUSH_CONST, std::monostate{});
    emit(OpCode::RETURN);
}

void IRGenerator::visitBreakStmt(BreakStmt& stmt) { (void)stmt; /* TODO: break logic */ }
void IRGenerator::visitContinueStmt(ContinueStmt& stmt) { (void)stmt; /* TODO: continue logic */ }
void IRGenerator::visitStructStmt(StructStmt& stmt) { (void)stmt; }
void IRGenerator::visitEnumStmt(EnumStmt& stmt) {
    for (size_t i = 0; i < stmt.members.size(); ++i) {
        emit(OpCode::PUSH_CONST, (int64_t)i);
        emit(OpCode::STORE_VAR, stmt.members[i].lexeme);
        emit(OpCode::POP);
    }
}
void IRGenerator::visitImportStmt(ImportStmt& stmt) { (void)stmt; }

void IRGenerator::visitInvalidStmt(InvalidStmt& stmt) {
    (void)stmt;
    emit(OpCode::NOP);
}

int IRGenerator::emitJump(OpCode op) {
    code_.emplace_back(op, Label{0});
    return code_.size() - 1;
}

void IRGenerator::patchJump(int idx) noexcept {
    code_[idx].operand = Label{(int64_t)code_.size()};
}

void IRGenerator::emit(OpCode op, IRValue val) {
    code_.emplace_back(op, std::move(val));
}

} // namespace vyronix