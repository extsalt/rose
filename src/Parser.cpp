#include "Parser.h"
#include <stdexcept>
#include <sstream>

namespace {
    struct ParserException : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
}

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

const Token& Parser::peek() const {
    if (current >= tokens.size()) return tokens.back();
    return tokens[current];
}

const Token& Parser::peekNext() const {
    if (current + 1 >= tokens.size()) return tokens.back();
    return tokens[current + 1];
}

const Token& Parser::previous() const {
    if (current == 0) return tokens.front();
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::Eof;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    recordError(peek(), message);
    throw ParserException(message);
}

void Parser::recordError(const Token& token, const std::string& message) {
    errors.push_back({token.location, message, token.text});
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;

        switch (peek().type) {
            case TokenType::Function:
            case TokenType::If:
            case TokenType::While:
            case TokenType::Return:
            case TokenType::Echo:
            case TokenType::RBrace:
                return;
            default:
                break;
        }

        advance();
    }
}

std::unique_ptr<ProgramAST> Parser::parse() {
    std::vector<std::unique_ptr<StmtAST>> statements;

    // Optional <?php tag
    if (match(TokenType::PhpOpenTag)) {
        // Tag consumed
    }

    while (!isAtEnd()) {
        if (match(TokenType::PhpCloseTag)) {
            break;
        }

        try {
            auto stmt = parseDeclarationOrStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const ParserException&) {
            synchronize();
        }
    }

    return std::make_unique<ProgramAST>(std::move(statements));
}

std::unique_ptr<StmtAST> Parser::parseDeclarationOrStatement() {
    if (check(TokenType::Function)) {
        return parseFunctionDeclaration();
    }
    return parseStatement();
}

std::unique_ptr<FunctionDeclAST> Parser::parseFunctionDeclaration() {
    consume(TokenType::Function, "Expected 'function' keyword.");
    const Token& nameToken = consume(TokenType::Identifier, "Expected function name.");
    std::string funcName = nameToken.text;

    consume(TokenType::LParen, "Expected '(' after function name.");
    std::vector<std::string> params;

    if (!check(TokenType::RParen)) {
        do {
            const Token& paramToken = consume(TokenType::Variable, "Expected parameter variable (e.g. '$x').");
            params.push_back(paramToken.text);
        } while (match(TokenType::Comma));
    }

    consume(TokenType::RParen, "Expected ')' after parameter list.");

    if (!check(TokenType::LBrace)) {
        recordError(peek(), "Expected '{' before function body.");
        throw ParserException("Expected '{' before function body.");
    }

    auto body = parseBlockStatement();
    return std::make_unique<FunctionDeclAST>(funcName, std::move(params), std::move(body));
}

std::unique_ptr<StmtAST> Parser::parseStatement() {
    if (check(TokenType::LBrace)) {
        return parseBlockStatement();
    }
    if (match(TokenType::If)) {
        return parseIfStatement();
    }
    if (match(TokenType::While)) {
        return parseWhileStatement();
    }
    if (match(TokenType::Return)) {
        return parseReturnStatement();
    }
    if (match(TokenType::Echo)) {
        return parseEchoStatement();
    }
    return parseExpressionStatement();
}

std::unique_ptr<BlockStmtAST> Parser::parseBlockStatement() {
    consume(TokenType::LBrace, "Expected '{'.");
    std::vector<std::unique_ptr<StmtAST>> statements;

    while (!check(TokenType::RBrace) && !isAtEnd()) {
        try {
            auto stmt = parseDeclarationOrStatement();
            if (stmt) {
                statements.push_back(std::move(stmt));
            }
        } catch (const ParserException&) {
            synchronize();
        }
    }

    consume(TokenType::RBrace, "Expected '}' after block.");
    return std::make_unique<BlockStmtAST>(std::move(statements));
}

std::unique_ptr<IfStmtAST> Parser::parseIfStatement() {
    consume(TokenType::LParen, "Expected '(' after 'if'.");
    auto condition = parseExpression();
    consume(TokenType::RParen, "Expected ')' after if condition.");

    auto thenBranch = parseStatement();
    std::unique_ptr<StmtAST> elseBranch = nullptr;

    if (match(TokenType::Else)) {
        elseBranch = parseStatement();
    }

    return std::make_unique<IfStmtAST>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<WhileStmtAST> Parser::parseWhileStatement() {
    consume(TokenType::LParen, "Expected '(' after 'while'.");
    auto condition = parseExpression();
    consume(TokenType::RParen, "Expected ')' after while condition.");

    auto body = parseStatement();
    return std::make_unique<WhileStmtAST>(std::move(condition), std::move(body));
}

std::unique_ptr<ReturnStmtAST> Parser::parseReturnStatement() {
    std::unique_ptr<ExprAST> expr = nullptr;
    if (!check(TokenType::Semicolon)) {
        expr = parseExpression();
    }
    consume(TokenType::Semicolon, "Expected ';' after return statement.");
    return std::make_unique<ReturnStmtAST>(std::move(expr));
}

std::unique_ptr<EchoStmtAST> Parser::parseEchoStatement() {
    std::vector<std::unique_ptr<ExprAST>> exprs;
    do {
        exprs.push_back(parseExpression());
    } while (match(TokenType::Comma));

    consume(TokenType::Semicolon, "Expected ';' after echo statement.");
    return std::make_unique<EchoStmtAST>(std::move(exprs));
}

std::unique_ptr<StmtAST> Parser::parseExpressionStatement() {
    auto expr = parseExpression();
    consume(TokenType::Semicolon, "Expected ';' after expression.");
    return std::make_unique<ExprStmtAST>(std::move(expr));
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<ExprAST> Parser::parseAssignment() {
    // Check if this looks like: $variable = expression
    if (check(TokenType::Variable) && peekNext().type == TokenType::Equal) {
        Token varToken = advance(); // Consume Variable
        advance();                  // Consume '='
        auto val = parseAssignment();
        return std::make_unique<AssignExprAST>(varToken.text, std::move(val));
    }
    return parseLogicalOr();
}

std::unique_ptr<ExprAST> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();

    while (match(TokenType::PipePipe)) {
        std::string op = previous().text;
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExprAST>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseLogicalAnd() {
    auto expr = parseEquality();

    while (match(TokenType::AmpAmp)) {
        std::string op = previous().text;
        auto right = parseEquality();
        expr = std::make_unique<BinaryExprAST>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseEquality() {
    auto expr = parseRelational();

    while (check(TokenType::EqualEqual) || check(TokenType::BangEqual)) {
        advance();
        std::string op = previous().text;
        auto right = parseRelational();
        expr = std::make_unique<BinaryExprAST>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseRelational() {
    auto expr = parseAdditive();

    while (check(TokenType::Less) || check(TokenType::LessEqual) ||
           check(TokenType::Greater) || check(TokenType::GreaterEqual)) {
        advance();
        std::string op = previous().text;
        auto right = parseAdditive();
        expr = std::make_unique<BinaryExprAST>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseAdditive() {
    auto expr = parseMultiplicative();

    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        advance();
        std::string op = previous().text;
        auto right = parseMultiplicative();
        expr = std::make_unique<BinaryExprAST>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseMultiplicative() {
    auto expr = parseUnary();

    while (check(TokenType::Star) || check(TokenType::Slash) || check(TokenType::Percent)) {
        advance();
        std::string op = previous().text;
        auto right = parseUnary();
        expr = std::make_unique<BinaryExprAST>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseUnary() {
    if (check(TokenType::Bang) || check(TokenType::Minus)) {
        advance();
        std::string op = previous().text;
        auto operand = parseUnary();
        return std::make_unique<UnaryExprAST>(op, std::move(operand));
    }
    return parsePrimary();
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    if (match(TokenType::IntLiteral)) {
        int64_t val = std::stoll(previous().text);
        return std::make_unique<IntExprAST>(val);
    }

    if (match(TokenType::FloatLiteral)) {
        double val = std::stod(previous().text);
        return std::make_unique<FloatExprAST>(val);
    }

    if (match(TokenType::StringLiteral)) {
        return std::make_unique<StringExprAST>(previous().text);
    }

    if (match(TokenType::BoolLiteral)) {
        bool val = (previous().text == "true" || previous().text == "TRUE");
        return std::make_unique<BoolExprAST>(val);
    }

    if (match(TokenType::NullLiteral)) {
        return std::make_unique<NullExprAST>();
    }

    if (match(TokenType::Variable)) {
        return std::make_unique<VariableExprAST>(previous().text);
    }

    if (match(TokenType::Identifier)) {
        std::string calleeName = previous().text;
        if (match(TokenType::LParen)) {
            std::vector<std::unique_ptr<ExprAST>> args;
            if (!check(TokenType::RParen)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::Comma));
            }
            consume(TokenType::RParen, "Expected ')' after function arguments.");
            return std::make_unique<CallExprAST>(calleeName, std::move(args));
        }

        recordError(previous(), "Unexpected identifier '" + calleeName + "'; expected variable or function call.");
        throw ParserException("Unexpected identifier");
    }

    if (match(TokenType::LParen)) {
        auto expr = parseExpression();
        consume(TokenType::RParen, "Expected ')' after grouped expression.");
        return expr;
    }

    recordError(peek(), "Unexpected token '" + peek().text + "' while parsing expression.");
    throw ParserException("Unexpected token in expression");
}
