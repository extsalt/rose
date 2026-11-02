#include "Token.h"

std::string_view tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Eof:           return "EOF";
        case TokenType::Invalid:       return "Invalid";
        case TokenType::PhpOpenTag:    return "<?php";
        case TokenType::PhpCloseTag:   return "?>";
        case TokenType::Variable:      return "Variable";
        case TokenType::Identifier:    return "Identifier";
        case TokenType::IntLiteral:    return "IntLiteral";
        case TokenType::FloatLiteral:  return "FloatLiteral";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::BoolLiteral:   return "BoolLiteral";
        case TokenType::NullLiteral:   return "NullLiteral";
        case TokenType::Function:      return "function";
        case TokenType::If:            return "if";
        case TokenType::Else:          return "else";
        case TokenType::While:         return "while";
        case TokenType::Return:        return "return";
        case TokenType::Echo:          return "echo";
        case TokenType::Plus:          return "+";
        case TokenType::Minus:         return "-";
        case TokenType::Star:          return "*";
        case TokenType::Slash:         return "/";
        case TokenType::Percent:       return "%";
        case TokenType::Equal:         return "=";
        case TokenType::EqualEqual:    return "==";
        case TokenType::BangEqual:     return "!=";
        case TokenType::Less:          return "<";
        case TokenType::LessEqual:     return "<=";
        case TokenType::Greater:       return ">";
        case TokenType::GreaterEqual:  return ">=";
        case TokenType::AmpAmp:        return "&&";
        case TokenType::PipePipe:      return "||";
        case TokenType::Bang:          return "!";
        case TokenType::Semicolon:     return ";";
        case TokenType::Comma:         return ",";
        case TokenType::LParen:        return "(";
        case TokenType::RParen:        return ")";
        case TokenType::LBrace:        return "{";
        case TokenType::RBrace:        return "}";
        case TokenType::LBracket:      return "[";
        case TokenType::RBracket:      return "]";
        default:                       return "Unknown";
    }
}
