#pragma once

#include "Token.h"
#include <vector>
#include <string>

class Lexer {
public:
    explicit Lexer(std::string source);

    std::vector<Token> tokenize();

private:
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    bool isAtEnd() const;

    void skipWhitespaceAndComments();
    Token scanToken();
    Token makeToken(TokenType type, std::string text);
    Token lexVariable();
    Token lexIdentifierOrKeyword();
    Token lexNumber();
    Token lexString(char quote);

    std::string source;
    size_t start = 0;
    size_t current = 0;
    size_t line = 1;
    size_t column = 1;
    size_t startColumn = 1;
};
