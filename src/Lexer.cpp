#include "Lexer.h"
#include <cctype>
#include <algorithm>

Lexer::Lexer(std::string source) : source(std::move(source)) {}

char Lexer::advance() {
    char c = source[current++];
    column++;
    return c;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.size()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    column++;
    return true;
}

bool Lexer::isAtEnd() const {
    return current >= source.size();
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
            line++;
            column = 1;
        } else if (c == '/' && peekNext() == '/') {
            // Line comment //
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else if (c == '#') {
            // Line comment #
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else if (c == '/' && peekNext() == '*') {
            // Block comment /* ... */
            advance(); // '/'
            advance(); // '*'
            while (!isAtEnd()) {
                if (peek() == '*' && peekNext() == '/') {
                    advance(); // '*'
                    advance(); // '/'
                    break;
                }
                if (peek() == '\n') {
                    advance();
                    line++;
                    column = 1;
                } else {
                    advance();
                }
            }
        } else {
            break;
        }
    }
}

Token Lexer::makeToken(TokenType type, std::string text) {
    Token token;
    token.type = type;
    token.text = std::move(text);
    token.location = {line, startColumn};
    return token;
}

Token Lexer::lexVariable() {
    // Current points right after '$'
    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }
    // Exclude the leading '$' so the token text contains only the variable name
    std::string text = source.substr(start + 1, current - (start + 1));
    return makeToken(TokenType::Variable, text);
}

Token Lexer::lexIdentifierOrKeyword() {
    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }
    std::string text = source.substr(start, current - start);

    // Convert to lowercase for keyword comparison (PHP keywords are case-insensitive)
    std::string lowerText = text;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    if (lowerText == "function") return makeToken(TokenType::Function, text);
    if (lowerText == "if")       return makeToken(TokenType::If, text);
    if (lowerText == "else")     return makeToken(TokenType::Else, text);
    if (lowerText == "while")    return makeToken(TokenType::While, text);
    if (lowerText == "return")   return makeToken(TokenType::Return, text);
    if (lowerText == "echo")     return makeToken(TokenType::Echo, text);
    if (lowerText == "true")     return makeToken(TokenType::BoolLiteral, text);
    if (lowerText == "false")    return makeToken(TokenType::BoolLiteral, text);
    if (lowerText == "null")     return makeToken(TokenType::NullLiteral, text);

    return makeToken(TokenType::Identifier, text);
}

Token Lexer::lexNumber() {
    bool isFloat = false;
    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }
    if (!isAtEnd() && peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        isFloat = true;
        advance(); // '.'
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }
    std::string text = source.substr(start, current - start);
    return makeToken(isFloat ? TokenType::FloatLiteral : TokenType::IntLiteral, text);
}

Token Lexer::lexString(char quote) {
    std::string value;
    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        if (peek() == '\\') {
            advance(); // '\'
            if (isAtEnd()) break;
            char escaped = advance();
            switch (escaped) {
                case 'n':  value.push_back('\n'); break;
                case 't':  value.push_back('\t'); break;
                case 'r':  value.push_back('\r'); break;
                case '\\': value.push_back('\\'); break;
                case '"':  value.push_back('"'); break;
                case '\'': value.push_back('\''); break;
                case '$':  value.push_back('$'); break;
                default:   value.push_back(escaped); break;
            }
        } else {
            value.push_back(advance());
        }
    }

    if (isAtEnd()) {
        return makeToken(TokenType::Invalid, "Unterminated string literal");
    }

    advance(); // Closing quote
    return makeToken(TokenType::StringLiteral, value);
}

Token Lexer::scanToken() {
    skipWhitespaceAndComments();
    start = current;
    startColumn = column;

    if (isAtEnd()) {
        return makeToken(TokenType::Eof, "");
    }

    // Check <?php
    if (peek() == '<' && source.compare(current, 5, "<?php") == 0) {
        current += 5;
        column += 5;
        return makeToken(TokenType::PhpOpenTag, "<?php");
    }

    // Check ?>
    if (peek() == '?' && peekNext() == '>') {
        advance(); // '?'
        advance(); // '>'
        return makeToken(TokenType::PhpCloseTag, "?>");
    }

    char c = advance();

    if (c == '$') {
        if (std::isalpha(static_cast<unsigned char>(peek())) || peek() == '_') {
            return lexVariable();
        }
        return makeToken(TokenType::Invalid, "$");
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return lexIdentifierOrKeyword();
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return lexNumber();
    }

    switch (c) {
        case '+': return makeToken(TokenType::Plus, "+");
        case '-': return makeToken(TokenType::Minus, "-");
        case '*': return makeToken(TokenType::Star, "*");
        case '/': return makeToken(TokenType::Slash, "/");
        case '%': return makeToken(TokenType::Percent, "%");
        case ';': return makeToken(TokenType::Semicolon, ";");
        case ',': return makeToken(TokenType::Comma, ",");
        case '(': return makeToken(TokenType::LParen, "(");
        case ')': return makeToken(TokenType::RParen, ")");
        case '{': return makeToken(TokenType::LBrace, "{");
        case '}': return makeToken(TokenType::RBrace, "}");
        case '[': return makeToken(TokenType::LBracket, "[");
        case ']': return makeToken(TokenType::RBracket, "]");
        case '=':
            if (match('=')) return makeToken(TokenType::EqualEqual, "==");
            return makeToken(TokenType::Equal, "=");
        case '!':
            if (match('=')) return makeToken(TokenType::BangEqual, "!=");
            return makeToken(TokenType::Bang, "!");
        case '<':
            if (match('=')) return makeToken(TokenType::LessEqual, "<=");
            return makeToken(TokenType::Less, "<");
        case '>':
            if (match('=')) return makeToken(TokenType::GreaterEqual, ">=");
            return makeToken(TokenType::Greater, ">");
        case '&':
            if (match('&')) return makeToken(TokenType::AmpAmp, "&&");
            return makeToken(TokenType::Invalid, "&");
        case '|':
            if (match('|')) return makeToken(TokenType::PipePipe, "||");
            return makeToken(TokenType::Invalid, "|");
        case '"':
        case '\'':
            return lexString(c);
        default:
            return makeToken(TokenType::Invalid, std::string(1, c));
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token token = scanToken();
        tokens.push_back(token);
        if (token.type == TokenType::Eof) {
            break;
        }
    }
    return tokens;
}
