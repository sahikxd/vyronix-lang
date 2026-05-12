#include "vyronix/SemanticAnalyzer.hpp"
#include "vyronix/Error.hpp"
#include "vyronix/NativeRegistry.hpp"
#include <iostream>
#include <algorithm>

namespace vyronix {

void SemanticAnalyzer::analyze(const std::vector<std::unique_ptr<Stmt>>& statements, std::string filename) {
    filename_ = std::move(filename);
    beginScope();

    SymbolInfo print_info;
    print_info.is_proc = true;
    print_info.type = std::make_unique<Type>(Type::Kind::BASE, "void");
    print_info.param_types.push_back(std::make_unique<Type>(Type::Kind::BASE, "any"));
    scopes_.back()["print"] = std::move(print_info);

    SymbolInfo pop_info;
    pop_info.is_function = true;
    pop_info.type = std::make_unique<Type>(Type::Kind::BASE, "any");
    pop_info.param_types.push_back(std::make_unique<Type>(std::make_unique<Type>(Type::Kind::BASE, "any")));
    pop_info.return_type = std::make_unique<Type>(Type::Kind::BASE, "any");
    scopes_.back()["pop"] = std::move(pop_info);

    SymbolInfo assert_info;
    assert_info.is_proc = true;
    assert_info.type = std::make_unique<Type>(Type::Kind::BASE, "void");
    assert_info.param_types.push_back(std::make_unique<Type>(Type::Kind::BASE, "bool"));
    scopes_.back()["assert"] = std::move(assert_info);

    SymbolInfo log_info;
    log_info.is_proc = true;
    log_info.type = std::make_unique<Type>(Type::Kind::BASE, "void");
    log_info.param_types.push_back(std::make_unique<Type>(Type::Kind::BASE, "any"));
    scopes_.back()["log"] = std::move(log_info);

    // --- Weak Namespace ---
    SymbolInfo weak_create_info;
    weak_create_info.is_function = true;
    weak_create_info.type = std::make_unique<Type>(Type::Kind::BASE, "any"); // Actually weak<any>
    weak_create_info.param_types.push_back(std::make_unique<Type>(Type::Kind::BASE, "any"));
    scopes_.back()["weak.create"] = std::move(weak_create_info);

    SymbolInfo weak_lock_info;
    weak_lock_info.is_function = true;
    weak_lock_info.type = std::make_unique<Type>(Type::Kind::BASE, "any");
    weak_lock_info.param_types.push_back(std::make_unique<Type>(Type::Kind::BASE, "any"));
    scopes_.back()["weak.lock"] = std::move(weak_lock_info);

    SymbolInfo weak_valid_info;
    weak_valid_info.is_function = true;
    weak_valid_info.type = std::make_unique<Type>(Type::Kind::BASE, "bool");
    weak_valid_info.param_types.push_back(std::make_unique<Type>(Type::Kind::BASE, "any"));
    scopes_.back()["weak.valid"] = std::move(weak_valid_info);

    for (const auto& stmt : statements) {
        if (!stmt) continue;
        stmt->accept(*this);
    }

    endScope();
}

void SemanticAnalyzer::visitBinaryExpr(BinaryExpr& expr) {
    if (!expr.left || !expr.right) {
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
        return;
    }
    expr.left->accept(*this);
    auto left_type = std::move(current_expr_type_);
    expr.right->accept(*this);
    auto right_type = std::move(current_expr_type_);

    if (!left_type || !right_type) {
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
        return;
    }

    if (expr.op.type == TokenType::EQ_EQ || expr.op.type == TokenType::BANG_EQ) {
        if (!checkTypes(*left_type, *right_type)) {
            error(expr.op, "Operands of equality comparison must be of same type. Got " + left_type->name + " and " + right_type->name);
        }
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "bool");
    } else if (expr.op.type == TokenType::LT || expr.op.type == TokenType::LT_EQ ||
               expr.op.type == TokenType::GT || expr.op.type == TokenType::GT_EQ) {
        if (!checkTypes(*left_type, *right_type) || left_type->name == "bool" || left_type->name == "str") {
            error(expr.op, "Operands of comparison must be numeric and of same type. Got " + left_type->name + " and " + right_type->name);
        }
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "bool");
    } else {
        if (!checkTypes(*left_type, *right_type)) {
            error(expr.op, "Operands of arithmetic must be of same type. Got " + left_type->name + " and " + right_type->name);
        }
        current_expr_type_ = left_type->clone();
    }
}

void SemanticAnalyzer::visitUnaryExpr(UnaryExpr& expr) {
    if (!expr.right) {
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
        return;
    }
    expr.right->accept(*this);
    if (current_expr_type_ && expr.op.type == TokenType::NOT) {
        if (current_expr_type_->name != "bool") error(expr.op, "Operand of 'not' must be bool.");
    } else if (current_expr_type_ && expr.op.type == TokenType::MINUS) {
        if (current_expr_type_->kind != Type::Kind::BASE || 
            (current_expr_type_->name[0] != 'i' && current_expr_type_->name[0] != 'u' && current_expr_type_->name[0] != 'f')) {
            error(expr.op, "Operand of '-' must be numeric.");
        }
    }
}

void SemanticAnalyzer::visitLiteralExpr(LiteralExpr& expr) {
    if (std::holds_alternative<int64_t>(expr.value)) current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "i64");
    else if (std::holds_alternative<double>(expr.value)) current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "f64");
    else if (std::holds_alternative<std::string>(expr.value)) current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "str");
    else if (std::holds_alternative<bool>(expr.value)) current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "bool");
    else if (std::holds_alternative<std::monostate>(expr.value)) current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
}

void SemanticAnalyzer::visitGroupingExpr(GroupingExpr& expr) {
    expr.expression->accept(*this);
}

void SemanticAnalyzer::visitClosureExpr(ClosureExpr& expr) {
    closure_stack_.push_back(&expr);
    closure_scope_depths_.push_back(scopes_.size());
    beginScope();
    auto old_return = std::move(current_function_return_type_);
    current_function_return_type_ = expr.return_type ? expr.return_type->clone() : std::make_unique<Type>(Type::Kind::BASE, "void");
    
    for (const auto& p : expr.params) {
        SymbolInfo p_info;
        p_info.type = p.type ? p.type->clone() : std::make_unique<Type>(Type::Kind::BASE, "any");
        p_info.is_const = false;
        declare(p.name, std::move(p_info));
    }
    
    if (expr.body) expr.body->accept(*this);
    
    current_function_return_type_ = std::move(old_return);
    endScope();
    closure_scope_depths_.pop_back();
    closure_stack_.pop_back();
    
    current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any"); 
}

void SemanticAnalyzer::visitVariableExpr(VariableExpr& expr) {
    auto [symbol, depth] = resolveWithDepth(expr.name.lexeme);
    if (!symbol) {
        // Check if it's a known namespace
        std::string name = expr.name.lexeme;
        if (name == "io" || name == "fs" || name == "str" || name == "arr" || name == "math" || name == "sys" || name == "weak") {
            current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
            return;
        }
        
        error(expr.name, "Undefined variable '" + expr.name.lexeme + "'.");
    } else {
        current_expr_type_ = symbol->type->clone();
        
        // Check for closure capture
        if (!closure_stack_.empty()) {
            // Is the variable declared outside the innermost closure?
            // depth is the index in scopes_ (0 is global)
            // closure_scope_depths_.back() is the scope index where the innermost closure started.
            
            if (depth < (int)closure_scope_depths_.back() && depth > 0) {
                // It's a capture (and not a global)
                expr.is_capture = true;
                
                // Add to capture list of all closures in the chain that need it
                for (int i = closure_stack_.size() - 1; i >= 0; --i) {
                    if (depth < (int)closure_scope_depths_[i]) {
                        auto& captured = closure_stack_[i]->captured_vars;
                        if (std::find(captured.begin(), captured.end(), expr.name.lexeme) == captured.end()) {
                            captured.push_back(expr.name.lexeme);
                        }
                    } else {
                        break; // Already in the scope of this closure or higher
                    }
                }
            }
        }
    }
}

void SemanticAnalyzer::visitAssignExpr(AssignExpr& expr) {
    auto [symbol, depth] = resolveWithDepth(expr.name.lexeme);
    if (!symbol) {
        error(expr.name, "Undefined variable '" + expr.name.lexeme + "'.");
    } else if (symbol->is_const) {
        error(expr.name, "Cannot assign to constant '" + expr.name.lexeme + "'.");
    } else {
        // Check for closure capture
        if (!closure_stack_.empty()) {
            if (depth < (int)closure_scope_depths_.back() && depth > 0) {
                expr.is_capture = true;
                for (int i = closure_stack_.size() - 1; i >= 0; --i) {
                    if (depth < (int)closure_scope_depths_[i]) {
                        auto& captured = closure_stack_[i]->captured_vars;
                        if (std::find(captured.begin(), captured.end(), expr.name.lexeme) == captured.end()) {
                            captured.push_back(expr.name.lexeme);
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }
    
    if (expr.value) {
        expr.value->accept(*this);
        if (symbol && current_expr_type_ && !checkTypes(*symbol->type, *current_expr_type_)) {
            error(expr.name, "Type mismatch in assignment.");
        }
    }
}

void SemanticAnalyzer::visitCallExpr(CallExpr& expr) {
    if (!expr.callee) {
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
        return;
    }
    expr.callee->accept(*this);
    auto callee_type = std::move(current_expr_type_);
    
    if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr.callee.get())) {
        auto symbol_opt = resolveWithDepth(v->name.lexeme).first;
        if (!symbol_opt) {
            error(v->name, "Undefined function or procedure '" + v->name.lexeme + "'.");
            current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
            return;
        }
        
        const auto& symbol = *symbol_opt;
        // std::cout << "[DEBUG] Calling '" << v->name.lexeme << "' type=" << symbol.type->name << " is_func=" << symbol.is_function << std::endl;
        
        // Allow calling if it's a function/proc OR if it's a variable that could hold a closure (type 'any')
        if (!symbol.is_function && !symbol.is_proc && symbol.type->name != "any") {
            error(v->name, "'" + v->name.lexeme + "' is not a function or procedure.");
            current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
            return;
        }
        
        // If it's a closure/any, we might not know the param types yet
        if (symbol.is_function || symbol.is_proc) {
            std::unordered_map<std::string, const Type*> substitutions;
            if (expr.type_args.size() > 0) {
                if (expr.type_args.size() != symbol.type_params.size()) {
                    error(expr.paren, "Type argument count mismatch for '" + v->name.lexeme + "'. Expected " + std::to_string(symbol.type_params.size()) + " but got " + std::to_string(expr.type_args.size()));
                } else {
                    for (size_t i = 0; i < symbol.type_params.size(); ++i) {
                        substitutions[symbol.type_params[i]] = expr.type_args[i].get();
                    }
                }
            }

            if (expr.arguments.size() != symbol.param_types.size()) {
                error(expr.paren, "Argument count mismatch for '" + v->name.lexeme + "'. Expected " + std::to_string(symbol.param_types.size()) + " but got " + std::to_string(expr.arguments.size()));
            }
            
            for (size_t i = 0; i < expr.arguments.size(); ++i) {
                if (expr.arguments[i]) {
                    expr.arguments[i]->accept(*this);
                    if (i < symbol.param_types.size() && current_expr_type_ && symbol.param_types[i]) {
                        auto expected_type = substituteTypes(*symbol.param_types[i], substitutions);
                        if (!checkTypes(*expected_type, *current_expr_type_)) {
                            error(expr.paren, "Argument " + std::to_string(i+1) + " type mismatch for '" + v->name.lexeme + "'. Expected " + expected_type->name + " but got " + current_expr_type_->name);
                        }
                    }
                }
            }

            if (symbol.is_function) {
                if (symbol.return_type) current_expr_type_ = substituteTypes(*symbol.return_type, substitutions);
                else current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
            } else {
                current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "void");
            }
        } else {
            // It's a variable call (closure)
            for (auto& arg : expr.arguments) if (arg) arg->accept(*this);
            current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
        }
    } else {
        // It's a complex callee (e.g. io.print() or (fn(x){x})(5))
        for (auto& arg : expr.arguments) if (arg) arg->accept(*this);
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
    }
}

void SemanticAnalyzer::visitLogicalExpr(LogicalExpr& expr) {
    if (expr.left) {
        expr.left->accept(*this);
        if (current_expr_type_ && current_expr_type_->name != "bool") error(expr.op, "Operand must be bool.");
    }
    if (expr.right) {
        expr.right->accept(*this);
        if (current_expr_type_ && current_expr_type_->name != "bool") error(expr.op, "Operand must be bool.");
    }
    current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "bool");
}

void SemanticAnalyzer::visitStructInstanceExpr(StructInstanceExpr& expr) {
    auto it = structs_.find(expr.struct_name.lexeme);
    if (it == structs_.end()) {
        error(expr.struct_name, "Undefined struct '" + expr.struct_name.lexeme + "'.");
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
        return;
    }
    
    // Check fields
    const auto& s_info = it->second;
    for (auto& init : expr.initializers) {
        auto f_it = s_info.fields.find(init.first.lexeme);
        if (f_it == s_info.fields.end()) {
            error(init.first, "Struct '" + expr.struct_name.lexeme + "' has no field '" + init.first.lexeme + "'.");
        } else if (init.second) {
            init.second->accept(*this);
            if (current_expr_type_ && !checkTypes(*f_it->second, *current_expr_type_)) {
                error(init.first, "Type mismatch for field '" + init.first.lexeme + "'.");
            }
        }
    }
    
    current_expr_type_ = std::make_unique<Type>(Type::Kind::CUSTOM, expr.struct_name.lexeme);
}

void SemanticAnalyzer::visitGetFieldExpr(GetFieldExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    auto obj_type = std::move(current_expr_type_);
    
    // Check for standard library namespaces
    if (auto var = dynamic_cast<VariableExpr*>(expr.object.get())) {
        std::string ns = var->name.lexeme;
        if (ns == "io" || ns == "fs" || ns == "str" || ns == "arr" || ns == "math" || ns == "sys" || ns == "weak") {
            std::string full_name = ns + "." + expr.name.lexeme;
            if (NativeRegistry::getInstance().exists(full_name)) {
                // It's a namespaced native function
                // We'll treat this as a special type for now or just allow it
                current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
                return;
            }
        }
    }
    
    if (obj_type && obj_type->kind == Type::Kind::CUSTOM) {
        auto it = structs_.find(obj_type->name);
        if (it != structs_.end()) {
            auto f_it = it->second.fields.find(expr.name.lexeme);
            if (f_it != it->second.fields.end()) {
                current_expr_type_ = f_it->second->clone();
                return;
            }
            error(expr.name, "Struct '" + obj_type->name + "' has no field '" + expr.name.lexeme + "'.");
        }
    }
    
    current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
}

void SemanticAnalyzer::visitSetFieldExpr(SetFieldExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    auto obj_type = std::move(current_expr_type_);
    
    if (expr.value) expr.value->accept(*this);
    auto val_type = std::move(current_expr_type_);
    
    if (obj_type && obj_type->kind == Type::Kind::CUSTOM) {
        auto it = structs_.find(obj_type->name);
        if (it != structs_.end()) {
            auto f_it = it->second.fields.find(expr.name.lexeme);
            if (f_it != it->second.fields.end()) {
                if (val_type && !checkTypes(*f_it->second, *val_type)) {
                    error(expr.name, "Type mismatch in field assignment.");
                }
                current_expr_type_ = f_it->second->clone();
                return;
            }
        }
    }
    
    current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
}

void SemanticAnalyzer::visitArrayLiteralExpr(ArrayLiteralExpr& expr) {
    std::unique_ptr<Type> element_type = nullptr;
    if (!expr.elements.empty()) {
        expr.elements[0]->accept(*this);
        element_type = current_expr_type_->clone();
        for (size_t i = 1; i < expr.elements.size(); ++i) {
            expr.elements[i]->accept(*this);
            if (!checkTypes(*element_type, *current_expr_type_)) {
                error(Token(TokenType::LBRACKET, "[", std::monostate{}, 0, 0), "Inconsistent array elements.");
            }
        }
    } else {
        element_type = std::make_unique<Type>(Type::Kind::BASE, "any");
    }
    current_expr_type_ = std::make_unique<Type>(std::move(element_type));
}

void SemanticAnalyzer::visitTupleExpr(TupleExpr& expr) {
    std::vector<std::unique_ptr<Type>> types;
    for (auto& element : expr.elements) {
        if (element) {
            element->accept(*this);
            types.push_back(current_expr_type_ ? current_expr_type_->clone() : std::make_unique<Type>(Type::Kind::BASE, "any"));
        } else {
            types.push_back(std::make_unique<Type>(Type::Kind::BASE, "any"));
        }
    }
    current_expr_type_ = std::make_unique<Type>(std::move(types));
}

void SemanticAnalyzer::visitIndexExpr(IndexExpr& expr) {
    expr.object->accept(*this);
    auto obj_type = std::move(current_expr_type_);
    
    expr.index->accept(*this);
    if (current_expr_type_->name != "i64" && current_expr_type_->name != "i32") {
        error(expr.bracket, "Index must be integer.");
    }
    
    if (obj_type->kind == Type::Kind::ARRAY) {
        current_expr_type_ = obj_type->element_type->clone();
    } else if (obj_type->kind == Type::Kind::TUPLE) {
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
    } else if (obj_type->name == "str") {
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "str");
    } else {
        error(expr.bracket, "Cannot index non-array/string.");
        current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
    }
}

void SemanticAnalyzer::visitSliceExpr(SliceExpr& expr) {
    expr.object->accept(*this);
    auto obj_type = std::move(current_expr_type_);
    
    if (expr.start) {
        expr.start->accept(*this);
        if (current_expr_type_->name != "i64" && current_expr_type_->name != "i32") error(expr.bracket, "Slice start must be int.");
    }
    if (expr.end) {
        expr.end->accept(*this);
        if (current_expr_type_->name != "i64" && current_expr_type_->name != "i32") error(expr.bracket, "Slice end must be int.");
    }
    
    current_expr_type_ = std::move(obj_type);
}

void SemanticAnalyzer::visitSetIndexExpr(SetIndexExpr& expr) {
    if (expr.object) expr.object->accept(*this);
    if (expr.index) expr.index->accept(*this);
    if (expr.value) {
        expr.value->accept(*this);
        current_expr_type_ = current_expr_type_ ? current_expr_type_->clone() : std::make_unique<Type>(Type::Kind::BASE, "any");
    }
}

void SemanticAnalyzer::visitInterpolatedStringExpr(InterpolatedStringExpr& expr) {
    for (auto& part : expr.parts) {
        if (std::holds_alternative<std::unique_ptr<Expr>>(part)) {
            std::get<std::unique_ptr<Expr>>(part)->accept(*this);
        }
    }
    current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "str");
}

void SemanticAnalyzer::visitInvalidExpr(InvalidExpr& expr) {
    (void)expr;
    current_expr_type_ = std::make_unique<Type>(Type::Kind::BASE, "any");
}

void SemanticAnalyzer::visitExpressionStmt(ExpressionStmt& stmt) {
    if (stmt.expression) stmt.expression->accept(*this);
}

void SemanticAnalyzer::visitVarStmt(VarStmt& stmt) {
    if (!stmt.type) {
        if (stmt.initializer) {
            stmt.type = inferType(*stmt.initializer);
        } else {
            error(stmt.name, "Variable needs type or initializer.");
            stmt.type = std::make_unique<Type>(Type::Kind::BASE, "any");
        }
    }
    
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        if (!checkTypes(*stmt.type, *current_expr_type_)) {
            error(stmt.name, "Type mismatch in variable initialization.");
        }
    }
    
    SymbolInfo info;
    info.type = stmt.type->clone();
    info.is_const = false;
    info.is_function = false;
    info.is_proc = false;
    declare(stmt.name, std::move(info));
}

void SemanticAnalyzer::visitConstStmt(ConstStmt& stmt) {
    if (!stmt.type) {
        stmt.type = inferType(*stmt.initializer);
    }
    
    stmt.initializer->accept(*this);
    if (!checkTypes(*stmt.type, *current_expr_type_)) {
        error(stmt.name, "Type mismatch in constant initialization.");
    }
    
    SymbolInfo info;
    info.type = stmt.type->clone();
    info.is_const = true;
    info.is_function = false;
    info.is_proc = false;
    declare(stmt.name, std::move(info));
}

void SemanticAnalyzer::visitBlockStmt(BlockStmt& stmt) {
    beginScope();
    for (const auto& s : stmt.statements) if (s) s->accept(*this);
    endScope();
}

void SemanticAnalyzer::visitIfStmt(IfStmt& stmt) {
    if (stmt.condition) {
        stmt.condition->accept(*this);
        if (current_expr_type_ && current_expr_type_->name != "bool") error(Token(TokenType::IF, "if", std::monostate{}, 0, 0), "If condition must be bool.");
    }
    if (stmt.thenBranch) stmt.thenBranch->accept(*this);
    for (auto& elif : stmt.elifBranches) {
        if (elif.condition) {
            elif.condition->accept(*this);
            if (current_expr_type_ && current_expr_type_->name != "bool") error(Token(TokenType::IF, "elif", std::monostate{}, 0, 0), "Elif condition must be bool.");
        }
        if (elif.body) elif.body->accept(*this);
    }
    if (stmt.elseBranch) stmt.elseBranch->accept(*this);
}

void SemanticAnalyzer::visitLoopStmt(LoopStmt& stmt) {
    if (stmt.condition) {
        stmt.condition->accept(*this);
        if (current_expr_type_ && current_expr_type_->name != "bool") error(Token(TokenType::LOOP, "loop", std::monostate{}, 0, 0), "Loop condition must be bool.");
    }
    if (stmt.body) stmt.body->accept(*this);
}

void SemanticAnalyzer::visitForStmt(ForStmt& stmt) {
    beginScope();
    if (stmt.start) {
        stmt.start->accept(*this);
        SymbolInfo iter_info;
        if (current_expr_type_) iter_info.type = current_expr_type_->clone();
        else iter_info.type = std::make_unique<Type>(Type::Kind::BASE, "i64");
        iter_info.is_const = false;
        declare(stmt.iterator, std::move(iter_info));
    }
    
    if (stmt.end) stmt.end->accept(*this);
    if (stmt.step) stmt.step->accept(*this);
    if (stmt.body) stmt.body->accept(*this);
    endScope();
}

void SemanticAnalyzer::visitMatchStmt(MatchStmt& stmt) {
    if (stmt.expression) {
        stmt.expression->accept(*this);
        auto match_type = std::move(current_expr_type_);
        for (auto& c : stmt.cases) {
            if (c.literal) {
                c.literal->accept(*this);
                if (match_type && current_expr_type_ && !checkTypes(*match_type, *current_expr_type_)) error(Token(TokenType::CASE, "case", std::monostate{}, 0, 0), "Case type mismatch.");
            }
            if (c.statement) c.statement->accept(*this);
        }
    }
}

void SemanticAnalyzer::visitTryStmt(TryStmt& stmt) {
    if (stmt.tryBlock) stmt.tryBlock->accept(*this);
    beginScope();
    SymbolInfo catch_info;
    catch_info.type = std::make_unique<Type>(Type::Kind::BASE, "str"); // Exceptions are strings for now
    declare(stmt.catchVar, std::move(catch_info));
    if (stmt.catchBlock) stmt.catchBlock->accept(*this);
    endScope();
}

void SemanticAnalyzer::visitThrowStmt(ThrowStmt& stmt) {
    if (stmt.expression) stmt.expression->accept(*this);
}

void SemanticAnalyzer::visitFunctionStmt(FunctionStmt& stmt) {
    SymbolInfo info;
    if (stmt.return_type) info.type = stmt.return_type->clone();
    else info.type = std::make_unique<Type>(Type::Kind::BASE, "void");
    info.is_const = true;
    info.is_function = true;
    info.is_proc = false;
    info.type_params = stmt.type_params;
    for (const auto& p : stmt.params) {
        info.param_types.push_back(p.type ? p.type->clone() : std::make_unique<Type>(Type::Kind::BASE, "any"));
    }
    if (stmt.return_type) info.return_type = stmt.return_type->clone();
    else info.return_type = std::make_unique<Type>(Type::Kind::BASE, "void");
    declare(stmt.name, std::move(info));

    std::set<std::string> t_params(stmt.type_params.begin(), stmt.type_params.end());
    type_param_stack_.push_back(std::move(t_params));

    beginScope();
    auto old_return = std::move(current_function_return_type_);
    if (stmt.return_type) current_function_return_type_ = stmt.return_type->clone();
    else current_function_return_type_ = std::make_unique<Type>(Type::Kind::BASE, "void");
    for (const auto& p : stmt.params) {
        SymbolInfo p_info;
        if (p.type) p_info.type = p.type->clone();
        else p_info.type = std::make_unique<Type>(Type::Kind::BASE, "any");
        p_info.is_const = false;
        declare(p.name, std::move(p_info));
    }
    if (stmt.body) stmt.body->accept(*this);
    current_function_return_type_ = std::move(old_return);
    endScope();

    type_param_stack_.pop_back();
}

void SemanticAnalyzer::visitProcStmt(ProcStmt& stmt) {
    SymbolInfo info;
    info.is_proc = true;
    info.type_params = stmt.type_params;
    for (const auto& p : stmt.params) {
        info.param_types.push_back(p.type ? p.type->clone() : std::make_unique<Type>(Type::Kind::BASE, "any"));
    }
    declare(stmt.name, std::move(info));
    
    std::set<std::string> t_params(stmt.type_params.begin(), stmt.type_params.end());
    type_param_stack_.push_back(std::move(t_params));
    
    beginScope();
    for (const auto& p : stmt.params) {
        SymbolInfo p_info;
        p_info.type = p.type ? p.type->clone() : std::make_unique<Type>(Type::Kind::BASE, "any");
        p_info.is_const = false;
        declare(p.name, std::move(p_info));
    }
    if (stmt.body) stmt.body->accept(*this);
    endScope();
    
    type_param_stack_.pop_back();
}

void SemanticAnalyzer::visitReturnStmt(ReturnStmt& stmt) {
    if (stmt.value) {
        stmt.value->accept(*this);
        if (current_function_return_type_ && current_expr_type_ && !checkTypes(*current_function_return_type_, *current_expr_type_)) error(stmt.keyword, "Return type mismatch.");
    } else {
        if (current_function_return_type_ && current_function_return_type_->name != "void") error(stmt.keyword, "Must return a value.");
    }
}

void SemanticAnalyzer::visitBreakStmt(BreakStmt& stmt) { (void)stmt; }
void SemanticAnalyzer::visitContinueStmt(ContinueStmt& stmt) { (void)stmt; }

void SemanticAnalyzer::visitStructStmt(StructStmt& stmt) {
    SymbolInfo info;
    info.is_struct = true;
    info.type = std::make_unique<Type>(Type::Kind::CUSTOM, stmt.name.lexeme);
    for (const auto& f : stmt.fields) {
        info.field_types[f.name.lexeme] = f.type ? f.type->clone() : std::make_unique<Type>(Type::Kind::BASE, "any");
    }
    declare(stmt.name, std::move(info));
    
    // Also register in structs_ map for member access checks
    StructInfo s_info;
    for (const auto& field : stmt.fields) {
        s_info.fields[field.name.lexeme] = field.type ? field.type->clone() : std::make_unique<Type>(Type::Kind::BASE, "any");
    }
    structs_[stmt.name.lexeme] = std::move(s_info);
}

void SemanticAnalyzer::visitEnumStmt(EnumStmt& stmt) {
    EnumInfo info;
    for (size_t i = 0; i < stmt.members.size(); ++i) {
        info.members[stmt.members[i].lexeme] = i;
        
        // Register member in current scope for direct access
        SymbolInfo member_info;
        member_info.is_const = true;
        member_info.type = std::make_unique<Type>(Type::Kind::BASE, "i64");
        declare(stmt.members[i], std::move(member_info));
    }
    enums_[stmt.name.lexeme] = std::move(info);
}

void SemanticAnalyzer::visitImportStmt(ImportStmt& stmt) { (void)stmt; }

void SemanticAnalyzer::visitInvalidStmt(InvalidStmt& stmt) { (void)stmt; }

void SemanticAnalyzer::beginScope() noexcept {
    scopes_.push_back({});
}

void SemanticAnalyzer::endScope() noexcept {
    scopes_.pop_back();
}

void SemanticAnalyzer::declare(const Token& name, SymbolInfo info) {
    if (scopes_.back().find(name.lexeme) != scopes_.back().end()) {
        error(name, "Already a symbol with this name in this scope.");
    }
    scopes_.back()[name.lexeme] = std::move(info);
}

std::optional<SymbolInfo> SemanticAnalyzer::resolve(const std::string& name) noexcept {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    return std::nullopt;
}

std::pair<std::optional<SymbolInfo>, int> SemanticAnalyzer::resolveWithDepth(const std::string& name) noexcept {
    for (int i = scopes_.size() - 1; i >= 0; --i) {
        auto found = scopes_[i].find(name);
        if (found != scopes_[i].end()) return {found->second, i};
    }
    return {std::nullopt, -1};
}

void SemanticAnalyzer::error(const Token& token, const std::string& message) {
    throw SemanticError(message, filename_, token.line, token.column);
}

std::unique_ptr<Type> SemanticAnalyzer::substituteTypes(const Type& type, const std::unordered_map<std::string, const Type*>& substitutions) {
    if (type.kind == Type::Kind::BASE || type.kind == Type::Kind::CUSTOM) {
        auto it = substitutions.find(type.name);
        if (it != substitutions.end()) {
            return it->second->clone();
        }
        return type.clone();
    }
    
    auto result = type.clone();
    if (result->element_type) {
        result->element_type = substituteTypes(*result->element_type, substitutions);
    }
    for (auto& tt : result->tuple_types) {
        if (tt) tt = substituteTypes(*tt, substitutions);
    }
    for (auto& ta : result->type_args) {
        if (ta) ta = substituteTypes(*ta, substitutions);
    }
    return result;
}

bool SemanticAnalyzer::checkTypes(const Type& left, const Type& right) noexcept {
    if (left.name == "any" || right.name == "any") return true;
    if (left.kind != right.kind) return false;
    if (left.kind == Type::Kind::BASE) return left.name == right.name;
    if (left.kind == Type::Kind::CUSTOM) return left.name == right.name;
    if (left.kind == Type::Kind::ARRAY) return checkTypes(*left.element_type, *right.element_type);
    if (left.kind == Type::Kind::TUPLE) {
        if (left.tuple_types.size() != right.tuple_types.size()) return false;
        for (size_t i = 0; i < left.tuple_types.size(); ++i) {
            if (!checkTypes(*left.tuple_types[i], *right.tuple_types[i])) return false;
        }
        return true;
    }
    return false;
}

std::unique_ptr<Type> SemanticAnalyzer::inferType(Expr& expr) {
    expr.accept(*this);
    return std::move(current_expr_type_);
}

} // namespace vyronix