#pragma once

#include "Token.h"
#include "AST.h"
#include <vector>
#include <memory>
#include <string>

struct ParseError {
    SourceLocation location;
    std::string message;
    std::string tokenText;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<ProgramAST> parse();

    bool hasErrors() const { return !errors.empty(); }
    const std::vector<ParseError>& getErrors() const { return errors; }

private:
    // Navigation & Inspection
    const Token& peek() const;
    const Token& peekNext() const;
    const Token& previous() const;
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token& advance();
    const Token& consume(TokenType type, const std::string& message);

    // Error handling & recovery
    void recordError(const Token& token, const std::string& message);
    void synchronize();

    // Declarations & Statements
    std::unique_ptr<StmtAST> parseDeclarationOrStatement();
    std::unique_ptr<FunctionDeclAST> parseFunctionDeclaration();
    std::unique_ptr<StmtAST> parseStatement();
    std::unique_ptr<BlockStmtAST> parseBlockStatement();
    std::unique_ptr<IfStmtAST> parseIfStatement();
    std::unique_ptr<WhileStmtAST> parseWhileStatement();
    std::unique_ptr<ReturnStmtAST> parseReturnStatement();
    std::unique_ptr<EchoStmtAST> parseEchoStatement();
    std::unique_ptr<StmtAST> parseExpressionStatement();

    // Expressions
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseAssignment();
    std::unique_ptr<ExprAST> parseLogicalOr();
    std::unique_ptr<ExprAST> parseLogicalAnd();
    std::unique_ptr<ExprAST> parseEquality();
    std::unique_ptr<ExprAST> parseRelational();
    std::unique_ptr<ExprAST> parseAdditive();
    std::unique_ptr<ExprAST> parseMultiplicative();
    std::unique_ptr<ExprAST> parseUnary();
    std::unique_ptr<ExprAST> parsePrimary();

    std::vector<Token> tokens;
    size_t current = 0;
    std::vector<ParseError> errors;
};
