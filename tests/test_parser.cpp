#include <iostream>
#include "vyronix/Lexer.hpp"
#include "vyronix/Parser.hpp"
#include "vyronix/AST.hpp"

using namespace vyronix;

class ASTPrinter : public ASTVisitor {
public:
    void print(ASTNode& node) {
        node.accept(*this);
    }

    void visitBinaryExpr(BinaryExpr& expr) override {
        std::cout << "(" << expr.op.lexeme << " ";
        expr.left->accept(*this);
        std::cout << " ";
        expr.right->accept(*this);
        std::cout << ")";
    }

    void visitUnaryExpr(UnaryExpr& expr) override {
        std::cout << "(" << expr.op.lexeme << " ";
        expr.right->accept(*this);
        std::cout << ")";
    }

    void visitLiteralExpr(LiteralExpr& expr) override {
        if (std::holds_alternative<int64_t>(expr.value)) {
            std::cout << std::get<int64_t>(expr.value);
        } else if (std::holds_alternative<double>(expr.value)) {
            std::cout << std::get<double>(expr.value);
        } else if (std::holds_alternative<std::string>(expr.value)) {
            std::cout << "\"" << std::get<std::string>(expr.value) << "\"";
        } else if (std::holds_alternative<bool>(expr.value)) {
            std::cout << (std::get<bool>(expr.value) ? "true" : "false");
        } else {
            std::cout << "null";
        }
    }

    void visitGroupingExpr(GroupingExpr& expr) override {
        std::cout << "(group ";
        expr.expression->accept(*this);
        std::cout << ")";
    }

    void visitVariableExpr(VariableExpr& expr) override {
        std::cout << expr.name.lexeme;
    }

    void visitAssignExpr(AssignExpr& expr) override {
        std::cout << "(assign " << expr.name.lexeme << " ";
        expr.value->accept(*this);
        std::cout << ")";
    }

    void visitCallExpr(CallExpr& expr) override {
        std::cout << "(call ";
        expr.callee->accept(*this);
        for (const auto& arg : expr.arguments) {
            std::cout << " ";
            arg->accept(*this);
        }
        std::cout << ")";
    }

    void visitLogicalExpr(LogicalExpr& expr) override {
        std::cout << "(" << expr.op.lexeme << " ";
        expr.left->accept(*this);
        std::cout << " ";
        expr.right->accept(*this);
        std::cout << ")";
    }

    void visitExpressionStmt(ExpressionStmt& stmt) override {
        stmt.expression->accept(*this);
        std::cout << ";" << std::endl;
    }

    void visitPrintStmt(PrintStmt& stmt) override {
        std::cout << "(print ";
        stmt.expression->accept(*this);
        std::cout << ");" << std::endl;
    }

    void visitVarStmt(VarStmt& stmt) override {
        std::cout << (stmt.is_const ? "(const " : "(let ") << stmt.name.lexeme;
        if (stmt.type_token.type != TokenType::T_VOID) {
            std::cout << ":" << stmt.type_token.lexeme;
        }
        if (stmt.initializer) {
            std::cout << " = ";
            stmt.initializer->accept(*this);
        }
        std::cout << ");" << std::endl;
    }

    void visitBlockStmt(BlockStmt& stmt) override {
        std::cout << "{" << std::endl;
        for (const auto& s : stmt.statements) {
            s->accept(*this);
        }
        std::cout << "}" << std::endl;
    }

    void visitIfStmt(IfStmt& stmt) override {
        std::cout << "(if ";
        stmt.condition->accept(*this);
        std::cout << " ";
        stmt.thenBranch->accept(*this);
        if (stmt.elseBranch) {
            std::cout << " else ";
            stmt.elseBranch->accept(*this);
        }
        std::cout << ")" << std::endl;
    }

    void visitWhileStmt(WhileStmt& stmt) override {
        std::cout << "(while ";
        stmt.condition->accept(*this);
        std::cout << " ";
        stmt.body->accept(*this);
        std::cout << ")" << std::endl;
    }

    void visitFunctionStmt(FunctionStmt& stmt) override {
        std::cout << "(fn " << stmt.name.lexeme << "(";
        for (size_t i = 0; i < stmt.params.size(); ++i) {
            std::cout << stmt.params[i].name.lexeme << ":" << stmt.params[i].type.lexeme;
            if (i < stmt.params.size() - 1) std::cout << ", ";
        }
        std::cout << ") -> " << stmt.return_type.lexeme << " ";
        stmt.body->accept(*this);
        std::cout << ")" << std::endl;
    }

    void visitReturnStmt(ReturnStmt& stmt) override {
        std::cout << "(return";
        if (stmt.value) {
            std::cout << " ";
            stmt.value->accept(*this);
        }
        std::cout << ");" << std::endl;
    }
};

int main() {
    std::string source = 
        "let x: int = 42;\n"
        "fn add(a: int, b: int) -> int {\n"
        "  return a + b;\n"
        "}\n"
        "if (x > 40) {\n"
        "  print(add(x, 1));\n"
        "}";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    ASTPrinter printer;
    for (const auto& stmt : statements) {
        printer.print(*stmt);
    }

    return 0;
}
