#pragma once

#include <string>
#include <string_view>
#include <cstddef>

enum class TokenType {
    // Special
    Eof,
    Invalid,
    PhpOpenTag,    // <?php
    PhpCloseTag,   // ?>

    // Identifiers & Variables
    Variable,      // Variable name without '$' (e.g. x, sum)
    Identifier,    // funcName

    // Literals
    IntLiteral,    // 42
    FloatLiteral,  // 3.14
    StringLiteral, // "hello"
    BoolLiteral,   // true, false
    NullLiteral,   // null

    // Keywords
    Function,      // function
    If,            // if
    Else,          // else
    While,         // while
    Return,        // return
    Echo,          // echo

    // Operators
    Plus,          // +
    Minus,         // -
    Star,          // *
    Slash,         // /
    Percent,       // %
    Equal,         // =
    EqualEqual,    // ==
    BangEqual,     // !=
    Less,          // <
    LessEqual,     // <=
    Greater,       // >
    GreaterEqual,  // >=
    AmpAmp,        // &&
    PipePipe,      // ||
    Bang,          // !

    // Punctuation & Delimiters
    Semicolon,     // ;
    Comma,         // ,
    LParen,        // (
    RParen,        // )
    LBrace,        // {
    RBrace,        // }
    LBracket,      // [
    RBracket       // ]
};

struct SourceLocation {
    size_t line = 1;
    size_t column = 1;
};

struct Token {
    TokenType type = TokenType::Invalid;
    std::string text;
    SourceLocation location;
};

std::string_view tokenTypeToString(TokenType type);
