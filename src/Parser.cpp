#include "vyronix/Parser.hpp"
#include "vyronix/Lexer.hpp"
#include "vyronix/Error.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace vyronix {

std::set<std::string> Parser::imported_files;
std::map<std::string, std::vector<std::unique_ptr<Stmt>>> Parser::module_cache;

Parser::Parser(std::vector<Token> tokens, std::string filename) noexcept
    : tokens_(std::move(tokens)), filename_(std::move(filename)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
        if (errorCount_ > maxErrors_) {
            std::cerr << "[FATAL] Too many errors. Aborting compilation." << std::endl;
            break;
        }
    }
    if (errorCount_ > 0) {
        throw SyntaxError("Parsing failed with " + std::to_string(errorCount_) + " errors.", filename_, 0, 0);
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
    try {
        if (panicMode_) synchronize();
        if (isAtEnd()) return nullptr;

        if (check(TokenType::IMPORT) || check(TokenType::USE) || check(TokenType::EXPORT)) return importDeclaration();
        if (check(TokenType::STRUCT)) return structDeclaration();
        if (check(TokenType::ENUM)) return enumDeclaration();
        if (check(TokenType::FN)) return functionDeclaration();
        if (check(TokenType::PROC)) return procDeclaration();
        if (check(TokenType::VAR) || check(TokenType::LET)) return variableDeclaration();
        if (check(TokenType::CONST)) return constDeclaration();
        
        auto stmt = statement();
        if (panicMode_) synchronize();
        return stmt;
    } catch (const SyntaxError& e) {
        // Error already reported by thrower
        synchronize();
        return std::make_unique<InvalidStmt>(e.what());
    }
}

std::unique_ptr<Stmt> Parser::variableDeclaration() {
    advance(); // Consume VAR or LET
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::unique_ptr<Type> type = nullptr;
    
    if (match({TokenType::COLON})) {
        type = parseType();
    }
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQ})) {
        initializer = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarStmt>(name, std::move(type), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::constDeclaration() {
    advance(); // Consume CONST
    Token name = consume(TokenType::IDENTIFIER, "Expect constant name.");
    std::unique_ptr<Type> type = nullptr;
    if (match({TokenType::COLON})) {
        type = parseType();
    }
    
    consume(TokenType::EQ, "Expect '=' for constant initialization.");
    std::unique_ptr<Expr> initializer = expression();
    
    consume(TokenType::SEMICOLON, "Expect ';' after constant declaration.");
    return std::make_unique<ConstStmt>(name, std::move(type), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::functionDeclaration() {
    advance(); // fn
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    
    std::vector<std::string> type_params;
    if (match({TokenType::LT})) {
        do {
            type_params.push_back(consume(TokenType::IDENTIFIER, "Expect type parameter name.").lexeme);
        } while (match({TokenType::COMMA}));
        consume(TokenType::GT, "Expect '>' after type parameters.");
    }

    consume(TokenType::LPAREN, "Expect '(' after function name.");
    
    std::vector<Parameter> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            Token p_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            consume(TokenType::COLON, "Expect ':' after parameter name.");
            std::unique_ptr<Type> p_type = parseType();
            parameters.push_back({p_name, std::move(p_type)});
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    
    std::unique_ptr<Type> return_type = nullptr;
    if (match({TokenType::ARROW})) {
        return_type = parseType();
    } else {
        return_type = std::make_unique<Type>(Type::Kind::BASE, "void");
    }
    
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<FunctionStmt>(name, std::move(parameters), std::move(return_type), std::move(body), std::move(type_params));
}

std::unique_ptr<Stmt> Parser::procDeclaration() {
    advance(); // Consume PROC
    Token name = consume(TokenType::IDENTIFIER, "Expect procedure name.");

    std::vector<std::string> type_params;
    if (match({TokenType::LT})) {
        do {
            type_params.push_back(consume(TokenType::IDENTIFIER, "Expect type parameter name.").lexeme);
        } while (match({TokenType::COMMA}));
        consume(TokenType::GT, "Expect '>' after type parameters.");
    }

    consume(TokenType::LPAREN, "Expect '(' after procedure name.");
    
    std::vector<Parameter> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            Token p_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            consume(TokenType::COLON, "Expect ':' after parameter name.");
            std::unique_ptr<Type> p_type = parseType();
            parameters.push_back({p_name, std::move(p_type)});
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<ProcStmt>(name, std::move(parameters), std::move(body), std::move(type_params));
}

std::unique_ptr<Stmt> Parser::structDeclaration() {
    advance(); // Consume STRUCT
    Token name = consume(TokenType::IDENTIFIER, "Expect struct name.");

    std::vector<std::string> type_params;
    if (match({TokenType::LT})) {
        do {
            type_params.push_back(consume(TokenType::IDENTIFIER, "Expect type parameter name.").lexeme);
        } while (match({TokenType::COMMA}));
        consume(TokenType::GT, "Expect '>' after type parameters.");
    }

    consume(TokenType::LBRACE, "Expect '{' before struct body.");
    
    std::vector<Parameter> fields;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        Token f_name = consume(TokenType::IDENTIFIER, "Expect field name.");
        consume(TokenType::COLON, "Expect ':' after field name.");
        std::unique_ptr<Type> f_type = parseType();
        consume(TokenType::SEMICOLON, "Expect ';' after field declaration.");
        fields.push_back({f_name, std::move(f_type)});
    }
    
    consume(TokenType::RBRACE, "Expect '}' after struct body.");
    return std::make_unique<StructStmt>(name, std::move(fields), std::move(type_params));
}

std::unique_ptr<Stmt> Parser::enumDeclaration() {
    advance(); // Consume ENUM
    Token name = consume(TokenType::IDENTIFIER, "Expect enum name.");
    consume(TokenType::LBRACE, "Expect '{' before enum body.");
    
    std::vector<Token> members;
    if (!check(TokenType::RBRACE)) {
        do {
            members.push_back(consume(TokenType::IDENTIFIER, "Expect enum member name."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RBRACE, "Expect '}' after enum body.");
    return std::make_unique<EnumStmt>(name, std::move(members));
}

std::unique_ptr<Stmt> Parser::importDeclaration() {
    bool is_export = check(TokenType::EXPORT);
    if (is_export) advance(); 
    
    if (match({TokenType::IMPORT})) {
        Token path_token = consume(TokenType::STRING_LIT, "Expect string literal for import path.");
        std::string import_path = std::get<std::string>(path_token.value);
        consume(TokenType::SEMICOLON, "Expect ';' after import.");

        // Resolve Path
        fs::path p(import_path);
        fs::path resolved_path;

        if (p.is_absolute()) {
            resolved_path = p;
        } else {
            // 1. Try relative to current file
            fs::path current_dir = fs::path(filename_).parent_path();
            resolved_path = current_dir / p;

            // 2. Try vx_modules/
            if (!fs::exists(resolved_path)) {
                resolved_path = fs::current_path() / "vx_modules" / import_path / "src" / "lib.vx";
            }
        }

        std::string final_path = resolved_path.string();
        if (imported_files.find(final_path) == imported_files.end()) {
            imported_files.insert(final_path);
            
            if (fs::exists(resolved_path)) {
                std::ifstream file(final_path);
                std::stringstream buffer;
                buffer << file.rdbuf();
                
                Lexer lexer(buffer.str(), final_path);
                auto tokens = lexer.tokenize();
                Parser parser(tokens, final_path);
                auto stmts = parser.parse();
                
                return std::make_unique<BlockStmt>(std::move(stmts));
            }
        }

        return std::make_unique<ImportStmt>(ImportStmt::Kind::STRING, import_path, is_export);
    } else if (match({TokenType::USE})) {
        consume(TokenType::MODULE, "Expect 'module' after 'use'.");
        Token module_name = consume(TokenType::IDENTIFIER, "Expect module name.");
        consume(TokenType::SEMICOLON, "Expect ';' after use module.");
        return std::make_unique<ImportStmt>(ImportStmt::Kind::MODULE, module_name.lexeme, is_export);
    }
    throw error(peek(), "Expect import or use.");
}

std::unique_ptr<Stmt> Parser::statement() {
    if (check(TokenType::IF)) { advance(); return ifStatement(); }
    if (check(TokenType::LOOP)) { advance(); return loopStatement(); }
    if (check(TokenType::FOR)) { advance(); return forStatement(); }
    if (check(TokenType::MATCH)) { advance(); return matchStatement(); }
    if (check(TokenType::TRY)) { advance(); return tryStatement(); }
    if (check(TokenType::THROW)) { advance(); return throwStatement(); }
    if (check(TokenType::RETURN)) { advance(); return returnStatement(); }
    if (check(TokenType::BREAK)) { advance(); return breakStatement(); }
    if (check(TokenType::CONTINUE)) { advance(); return continueStatement(); }
    if (check(TokenType::LBRACE)) return block();
    
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after condition.");
    std::unique_ptr<BlockStmt> thenBranch = block();
    
    std::vector<IfStmt::ElifBranch> elifs;
    while (match({TokenType::ELIF})) {
        consume(TokenType::LPAREN, "Expect '(' after 'elif'.");
        std::unique_ptr<Expr> elifCond = expression();
        consume(TokenType::RPAREN, "Expect ')' after elif condition.");
        std::unique_ptr<BlockStmt> elifBody = block();
        elifs.push_back({std::move(elifCond), std::move(elifBody)});
    }
    
    std::unique_ptr<BlockStmt> elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = block();
    }
    
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elifs), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::loopStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'loop'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after condition.");
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<LoopStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::forStatement() {
    Token iterator = consume(TokenType::IDENTIFIER, "Expect iterator name.");
    consume(TokenType::EQ, "Expect '=' after iterator.");
    std::unique_ptr<Expr> start = expression();
    consume(TokenType::TO, "Expect 'to' after start value.");
    std::unique_ptr<Expr> end = expression();
    
    std::unique_ptr<Expr> step = nullptr;
    if (match({TokenType::STEP})) {
        step = expression();
    }
    
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<ForStmt>(iterator, std::move(start), std::move(end), std::move(step), std::move(body));
}

std::unique_ptr<Stmt> Parser::matchStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'match'.");
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::RPAREN, "Expect ')' after match expression.");
    
    consume(TokenType::LBRACE, "Expect '{' before match cases.");
    std::vector<MatchCase> cases;
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match({TokenType::CASE})) {
            std::unique_ptr<Expr> literal = expression();
            consume(TokenType::COLON, "Expect ':' after case literal.");
            std::unique_ptr<Stmt> stmt = statement();
            cases.push_back({std::move(literal), std::move(stmt)});
        } else if (match({TokenType::DEFAULT})) {
            consume(TokenType::COLON, "Expect ':' after default.");
            std::unique_ptr<Stmt> stmt = statement();
            cases.push_back({nullptr, std::move(stmt)});
        } else {
            error(peek(), "Expect 'case' or 'default' in match block.");
            synchronize();
        }
    }
    consume(TokenType::RBRACE, "Expect '}' after match cases.");
    return std::make_unique<MatchStmt>(std::move(expr), std::move(cases));
}

std::unique_ptr<Stmt> Parser::tryStatement() {
    std::unique_ptr<BlockStmt> tryBlock = block();
    consume(TokenType::CATCH, "Expect 'catch' after try block.");
    consume(TokenType::LPAREN, "Expect '(' after 'catch'.");
    Token catchVar = consume(TokenType::IDENTIFIER, "Expect catch variable name.");
    consume(TokenType::RPAREN, "Expect ')' after catch variable.");
    std::unique_ptr<BlockStmt> catchBlock = block();
    return std::make_unique<TryStmt>(std::move(tryBlock), catchVar, std::move(catchBlock));
}

std::unique_ptr<Stmt> Parser::throwStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after throw statement.");
    return std::make_unique<ThrowStmt>(keyword, std::move(expr));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON) && !check(TokenType::RBRACE)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<Stmt> Parser::breakStatement() {
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expect ';' after break.");
    return std::make_unique<BreakStmt>(keyword);
}

std::unique_ptr<Stmt> Parser::continueStatement() {
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "Expect ';' after continue.");
    return std::make_unique<ContinueStmt>(keyword);
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<BlockStmt> Parser::block() {
    consume(TokenType::LBRACE, "Expect '{' to start block.");
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = declaration();
        if (stmt) statements.push_back(std::move(stmt));
    }
    consume(TokenType::RBRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Expr> Parser::expression() {
    try {
        return assignment();
    } catch (const SyntaxError& e) {
        error(peek(), e.what());
        synchronize();
        return std::make_unique<InvalidExpr>(e.what());
    }
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = logical_or();

    if (match({TokenType::EQ})) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment();

        if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = v->name;
            return std::make_unique<AssignExpr>(name, std::move(value));
        } else if (IndexExpr* i = dynamic_cast<IndexExpr*>(expr.get())) {
            return std::make_unique<SetIndexExpr>(std::move(i->object), std::move(i->index), std::move(value));
        } else if (GetFieldExpr* g = dynamic_cast<GetFieldExpr*>(expr.get())) {
            return std::make_unique<SetFieldExpr>(std::move(g->object), g->name, std::move(value));
        }

        error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expr> Parser::logical_or() {
    std::unique_ptr<Expr> expr = logical_and();
    while (match({TokenType::OR})) {
        Token op = previous();
        std::unique_ptr<Expr> right = logical_and();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logical_and() {
    std::unique_ptr<Expr> expr = equality();
    while (match({TokenType::AND})) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();
    while (match({TokenType::EQ_EQ, TokenType::BANG_EQ})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();
    while (match({TokenType::LT, TokenType::GT, TokenType::LT_EQ, TokenType::GT_EQ})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();
    while (match({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>(true);
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>(false);
    if (match({TokenType::NULL_TOKEN})) return std::make_unique<LiteralExpr>(std::monostate{});
    if (match({TokenType::INT_LIT, TokenType::FLOAT_LIT, TokenType::STRING_LIT})) return std::make_unique<LiteralExpr>(previous().value);
    
    if (match({TokenType::FN})) {
        consume(TokenType::LPAREN, "Expect '(' after 'fn'.");
        std::vector<Parameter> parameters;
        if (!check(TokenType::RPAREN)) {
            do {
                Token p_name = consume(TokenType::IDENTIFIER, "Expect parameter name.");
                consume(TokenType::COLON, "Expect ':' after parameter name.");
                std::unique_ptr<Type> p_type = parseType();
                parameters.push_back({p_name, std::move(p_type)});
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RPAREN, "Expect ')' after parameters.");
        
        std::unique_ptr<Type> return_type = nullptr;
        if (match({TokenType::ARROW})) {
            return_type = parseType();
        } else {
            return_type = std::make_unique<Type>(Type::Kind::BASE, "void");
        }
        
        std::unique_ptr<BlockStmt> body = block();
        return std::make_unique<ClosureExpr>(std::move(parameters), std::move(return_type), std::move(body));
    }

    if (match({TokenType::CALL})) {
        Token name = consume(TokenType::IDENTIFIER, "Expect function name after 'call'.");
        std::unique_ptr<Expr> expr = std::make_unique<VariableExpr>(name);
        consume(TokenType::LPAREN, "Expect '(' after function name.");
        return finishCall(std::move(expr), {});
    }

    if (match({TokenType::IDENTIFIER}) || match({TokenType::STR}) || match({TokenType::ARRAY}) || 
        match({TokenType::WEAK}) || match({TokenType::I64}) || match({TokenType::F64}) || match({TokenType::BOOL})) {
        Token name = previous();
        
        // If it was a keyword, we treat it as an identifier for namespacing/member access
        if (name.type != TokenType::IDENTIFIER) {
            name.type = TokenType::IDENTIFIER;
        }
        
        // Check for struct instantiation
        if (check(TokenType::LBRACE)) {
            advance(); // consume '{'
            std::vector<std::pair<Token, std::unique_ptr<Expr>>> initializers;
            if (!check(TokenType::RBRACE)) {
                do {
                    Token f_name = consume(TokenType::IDENTIFIER, "Expect field name.");
                    consume(TokenType::COLON, "Expect ':' after field name.");
                    std::unique_ptr<Expr> value = expression();
                    initializers.push_back({f_name, std::move(value)});
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RBRACE, "Expect '}' after struct initializers.");
            return std::make_unique<StructInstanceExpr>(name, std::move(initializers));
        }

        std::unique_ptr<Expr> expr = std::make_unique<VariableExpr>(name);
        
        while (check(TokenType::LPAREN) || check(TokenType::LBRACKET) || check(TokenType::DOT) || check(TokenType::LT)) {
            if (match({TokenType::LPAREN})) {
                expr = finishCall(std::move(expr), {});
            } else if (match({TokenType::LT})) {
                // Potential generic: identity<i64>(...)
                // We need to be careful not to consume < if it's a comparison.
                // One way is to check if it's followed by a type and then eventually a > and (.
                // For now, let's assume it's a generic if we are in this loop.
                std::vector<std::unique_ptr<Type>> t_args;
                do {
                    t_args.push_back(parseType());
                } while (match({TokenType::COMMA}));
                consume(TokenType::GT, "Expect '>' after type arguments.");
                
                if (match({TokenType::LPAREN})) {
                    expr = finishCall(std::move(expr), std::move(t_args));
                } else {
                    // It might be a generic struct instantiation: Box<i64> { value: 5 }
                    if (match({TokenType::LBRACE})) {
                        // Handle generic struct instantiation
                        std::vector<std::pair<Token, std::unique_ptr<Expr>>> initializers;
                        if (!check(TokenType::RBRACE)) {
                            do {
                                Token f_name = consume(TokenType::IDENTIFIER, "Expect field name.");
                                consume(TokenType::COLON, "Expect ':' after field name.");
                                std::unique_ptr<Expr> value = expression();
                                initializers.push_back({f_name, std::move(value)});
                            } while (match({TokenType::COMMA}));
                        }
                        consume(TokenType::RBRACE, "Expect '}' after struct initializers.");
                        // We need a way to store type_args in StructInstanceExpr
                        // Let's just return a VariableExpr for now or update StructInstanceExpr
                        expr = std::make_unique<StructInstanceExpr>(name, std::move(initializers));
                        // TODO: Add type_args to StructInstanceExpr
                    } else {
                        error(peek(), "Expect '(' or '{' after type arguments.");
                    }
                }
            } else if (match({TokenType::LBRACKET})) {
                expr = arrayAccess(std::move(expr));
            } else if (match({TokenType::DOT})) {
                Token name = consume(TokenType::IDENTIFIER, "Expect field name after '.'.");
                expr = std::make_unique<GetFieldExpr>(std::move(expr), name);
            }
        }
        return expr;
    }
    
    if (match({TokenType::LPAREN})) {
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RPAREN)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        
        if (elements.size() == 1 && previous().type != TokenType::COMMA) {
            // Grouping if single element and no trailing comma
            // Wait, usually (x) is grouping, (x,) is tuple of 1.
            // But let's check if there was a comma.
            // Actually, if elements.size() == 1, it's a grouping unless there was a comma.
            // However, our `do-while` loop consumed commas.
            // Let's check if we matched at least one comma.
            // Actually, a simpler way:
            return std::make_unique<GroupingExpr>(std::move(elements[0]));
        }
        return std::make_unique<TupleExpr>(std::move(elements));
    }
    
    if (match({TokenType::INTERPOLATION_START})) return interpolatedString();
    
    if (match({TokenType::LBRACKET})) {
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RBRACKET, "Expect ']' after array literal.");
        return std::make_unique<ArrayLiteralExpr>(std::move(elements));
    }
    
    throw error(peek(), "Expect expression.");
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Type>> type_args) {
    std::vector<std::unique_ptr<Expr>> arguments;
    if (!check(TokenType::RPAREN)) {
        do {
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    Token paren = consume(TokenType::RPAREN, "Expect ')' after arguments.");
    
    return std::make_unique<CallExpr>(std::move(callee), paren, std::move(arguments), std::move(type_args));
}

std::unique_ptr<Expr> Parser::arrayAccess(std::unique_ptr<Expr> object) {
    Token bracket = previous();
    std::unique_ptr<Expr> first = nullptr;
    if (!check(TokenType::COLON)) {
        first = expression();
    }
    
    if (match({TokenType::COLON})) {
        std::unique_ptr<Expr> second = nullptr;
        if (!check(TokenType::RBRACKET)) {
            second = expression();
        }
        consume(TokenType::RBRACKET, "Expect ']' after slice.");
        return std::make_unique<SliceExpr>(std::move(object), bracket, std::move(first), std::move(second));
    } else {
        consume(TokenType::RBRACKET, "Expect ']' after index.");
        return std::make_unique<IndexExpr>(std::move(object), bracket, std::move(first));
    }
}

std::unique_ptr<Expr> Parser::interpolatedString() {
    std::vector<std::variant<std::string, std::unique_ptr<Expr>>> parts;
    while (!match({TokenType::INTERPOLATION_END})) {
        if (match({TokenType::STRING_LIT})) {
            parts.push_back(std::get<std::string>(previous().value));
        } else if (match({TokenType::LBRACE})) {
            parts.push_back(expression());
            consume(TokenType::RBRACE, "Expect '}' after expression in interpolation.");
        } else {
            throw error(peek(), "Unexpected token in string interpolation.");
        }
    }
    return std::make_unique<InterpolatedStringExpr>(std::move(parts));
}

std::unique_ptr<Type> Parser::parseType() {
    if (match({TokenType::I8, TokenType::I16, TokenType::I32, TokenType::I64,
               TokenType::U8, TokenType::U16, TokenType::U32, TokenType::U64,
               TokenType::F32, TokenType::F64, TokenType::BOOL, TokenType::STR,
               TokenType::VOID, TokenType::ANY})) {
        std::string name;
        switch (previous().type) {
            case TokenType::I8: name = "i8"; break;
            case TokenType::I16: name = "i16"; break;
            case TokenType::I32: name = "i32"; break;
            case TokenType::I64: name = "i64"; break;
            case TokenType::U8: name = "u8"; break;
            case TokenType::U16: name = "u16"; break;
            case TokenType::U32: name = "u32"; break;
            case TokenType::U64: name = "u64"; break;
            case TokenType::F32: name = "f32"; break;
            case TokenType::F64: name = "f64"; break;
            case TokenType::BOOL: name = "bool"; break;
            case TokenType::STR: name = "str"; break;
            case TokenType::VOID: name = "void"; break;
            case TokenType::ANY: name = "any"; break;
            default: name = "any"; break;
        }
        return std::make_unique<Type>(Type::Kind::BASE, name);
    }
    
    if (match({TokenType::ARRAY})) {
        consume(TokenType::LT, "Expect '<' after 'array'.");
        auto elem_type = parseType();
        consume(TokenType::GT, "Expect '>' after array element type.");
        return std::make_unique<Type>(std::move(elem_type));
    }

    if (match({TokenType::WEAK})) {
        consume(TokenType::LT, "Expect '<' after 'weak'.");
        auto elem_type = parseType();
        consume(TokenType::GT, "Expect '>' after weak element type.");
        return std::make_unique<Type>(Type::Kind::CUSTOM, "weak<" + elem_type->getName() + ">");
    }

    if (match({TokenType::LPAREN})) {
        std::vector<std::unique_ptr<Type>> types;
        if (!check(TokenType::RPAREN)) {
            do {
                types.push_back(parseType());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RPAREN, "Expect ')' after tuple type.");
        return std::make_unique<Type>(std::move(types));
    }
    
    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<Type>(Type::Kind::CUSTOM, previous().lexeme);
    }
    
    throw error(peek(), "Expect type.");
}

bool Parser::match(std::initializer_list<TokenType> types) noexcept {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const noexcept {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() noexcept {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::isAtEnd() const noexcept {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::peek() const noexcept {
    return tokens_[current_];
}

Token Parser::previous() const noexcept {
    return tokens_[current_ - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

void Parser::synchronize() noexcept {
    panicMode_ = false;
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::STRUCT:
            case TokenType::ENUM:
            case TokenType::FN:
            case TokenType::PROC:
            case TokenType::VAR:
            case TokenType::LET:
            case TokenType::CONST:
            case TokenType::IF:
            case TokenType::LOOP:
            case TokenType::FOR:
            case TokenType::MATCH:
            case TokenType::RETURN:
                return;
            default: break;
        }
        advance();
    }
}

SyntaxError Parser::error(const Token& token, const std::string& message) noexcept {
    errorCount_++;
    std::cerr << "[SYNTAX ERROR] " << filename_ << ":" << token.line << ":" << token.column << ": " << message << std::endl;
    panicMode_ = true;
    return SyntaxError(message, filename_, token.line, token.column);
}

} // namespace vyronix