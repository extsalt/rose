#include "test_framework.h"
#include "Lexer.h"

TEST_CASE("Lexer: Variable Name Strips Leading Dollar") {
    Lexer lexer("$x $_test123 $total $a_b_c");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), 5u); // 4 variables + EOF
    ASSERT_EQ(tokens[0].type, TokenType::Variable);
    ASSERT_EQ(tokens[0].text, "x");

    ASSERT_EQ(tokens[1].type, TokenType::Variable);
    ASSERT_EQ(tokens[1].text, "_test123");

    ASSERT_EQ(tokens[2].type, TokenType::Variable);
    ASSERT_EQ(tokens[2].text, "total");

    ASSERT_EQ(tokens[3].type, TokenType::Variable);
    ASSERT_EQ(tokens[3].text, "a_b_c");

    ASSERT_EQ(tokens[4].type, TokenType::Eof);
}

TEST_CASE("Lexer: Keywords Case-Insensitive Recognition") {
    Lexer lexer("function FUNCTION Function if IF Else WHILE return RETURN echo ECHO");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::Function);
    ASSERT_EQ(tokens[1].type, TokenType::Function);
    ASSERT_EQ(tokens[2].type, TokenType::Function);
    ASSERT_EQ(tokens[3].type, TokenType::If);
    ASSERT_EQ(tokens[4].type, TokenType::If);
    ASSERT_EQ(tokens[5].type, TokenType::Else);
    ASSERT_EQ(tokens[6].type, TokenType::While);
    ASSERT_EQ(tokens[7].type, TokenType::Return);
    ASSERT_EQ(tokens[8].type, TokenType::Return);
    ASSERT_EQ(tokens[9].type, TokenType::Echo);
    ASSERT_EQ(tokens[10].type, TokenType::Echo);
    ASSERT_EQ(tokens[11].type, TokenType::Eof);
}

TEST_CASE("Lexer: Integer and Float Literals") {
    Lexer lexer("0 42 1000 3.14 0.5 99.999");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::IntLiteral);
    ASSERT_EQ(tokens[0].text, "0");

    ASSERT_EQ(tokens[1].type, TokenType::IntLiteral);
    ASSERT_EQ(tokens[1].text, "42");

    ASSERT_EQ(tokens[2].type, TokenType::IntLiteral);
    ASSERT_EQ(tokens[2].text, "1000");

    ASSERT_EQ(tokens[3].type, TokenType::FloatLiteral);
    ASSERT_EQ(tokens[3].text, "3.14");

    ASSERT_EQ(tokens[4].type, TokenType::FloatLiteral);
    ASSERT_EQ(tokens[4].text, "0.5");

    ASSERT_EQ(tokens[5].type, TokenType::FloatLiteral);
    ASSERT_EQ(tokens[5].text, "99.999");
}

TEST_CASE("Lexer: String Literals and Escape Sequences") {
    Lexer lexer("\"hello world\" 'single quote' \"line1\\nline2\\t\\\"escaped\\\"\\\\slash\\$dollar\"");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::StringLiteral);
    ASSERT_EQ(tokens[0].text, "hello world");

    ASSERT_EQ(tokens[1].type, TokenType::StringLiteral);
    ASSERT_EQ(tokens[1].text, "single quote");

    ASSERT_EQ(tokens[2].type, TokenType::StringLiteral);
    ASSERT_EQ(tokens[2].text, "line1\nline2\t\"escaped\"\\slash$dollar");
}

TEST_CASE("Lexer: Boolean and Null Literals") {
    Lexer lexer("true false TRUE FALSE null NULL");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::BoolLiteral);
    ASSERT_EQ(tokens[0].text, "true");

    ASSERT_EQ(tokens[1].type, TokenType::BoolLiteral);
    ASSERT_EQ(tokens[1].text, "false");

    ASSERT_EQ(tokens[2].type, TokenType::BoolLiteral);
    ASSERT_EQ(tokens[2].text, "TRUE");

    ASSERT_EQ(tokens[3].type, TokenType::BoolLiteral);
    ASSERT_EQ(tokens[3].text, "FALSE");

    ASSERT_EQ(tokens[4].type, TokenType::NullLiteral);
    ASSERT_EQ(tokens[4].text, "null");

    ASSERT_EQ(tokens[5].type, TokenType::NullLiteral);
    ASSERT_EQ(tokens[5].text, "NULL");
}

TEST_CASE("Lexer: Operators and Delimiters") {
    Lexer lexer("+ - * / % = == != < <= > >= && || ! ; , ( ) { } [ ]");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::Plus);
    ASSERT_EQ(tokens[1].type, TokenType::Minus);
    ASSERT_EQ(tokens[2].type, TokenType::Star);
    ASSERT_EQ(tokens[3].type, TokenType::Slash);
    ASSERT_EQ(tokens[4].type, TokenType::Percent);
    ASSERT_EQ(tokens[5].type, TokenType::Equal);
    ASSERT_EQ(tokens[6].type, TokenType::EqualEqual);
    ASSERT_EQ(tokens[7].type, TokenType::BangEqual);
    ASSERT_EQ(tokens[8].type, TokenType::Less);
    ASSERT_EQ(tokens[9].type, TokenType::LessEqual);
    ASSERT_EQ(tokens[10].type, TokenType::Greater);
    ASSERT_EQ(tokens[11].type, TokenType::GreaterEqual);
    ASSERT_EQ(tokens[12].type, TokenType::AmpAmp);
    ASSERT_EQ(tokens[13].type, TokenType::PipePipe);
    ASSERT_EQ(tokens[14].type, TokenType::Bang);
    ASSERT_EQ(tokens[15].type, TokenType::Semicolon);
    ASSERT_EQ(tokens[16].type, TokenType::Comma);
    ASSERT_EQ(tokens[17].type, TokenType::LParen);
    ASSERT_EQ(tokens[18].type, TokenType::RParen);
    ASSERT_EQ(tokens[19].type, TokenType::LBrace);
    ASSERT_EQ(tokens[20].type, TokenType::RBrace);
    ASSERT_EQ(tokens[21].type, TokenType::LBracket);
    ASSERT_EQ(tokens[22].type, TokenType::RBracket);
}

TEST_CASE("Lexer: Comment Stripping (//, #, /* */)") {
    std::string source =
        "// C++ style comment\n"
        "$x = 1;\n"
        "# Shell style comment\n"
        "$y = 2;\n"
        "/* Block comment\n"
        "   across multiple lines */\n"
        "$z = 3;";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // Should only yield: $x, =, 1, ;, $y, =, 2, ;, $z, =, 3, ;, EOF
    ASSERT_EQ(tokens.size(), 13u);
    ASSERT_EQ(tokens[0].type, TokenType::Variable);
    ASSERT_EQ(tokens[0].text, "x");
    ASSERT_EQ(tokens[4].type, TokenType::Variable);
    ASSERT_EQ(tokens[4].text, "y");
    ASSERT_EQ(tokens[8].type, TokenType::Variable);
    ASSERT_EQ(tokens[8].text, "z");
    ASSERT_EQ(tokens[12].type, TokenType::Eof);
}

TEST_CASE("Lexer: PHP Open and Close Tags") {
    Lexer lexer("<?php $a = 10; ?>");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::PhpOpenTag);
    ASSERT_EQ(tokens[0].text, "<?php");

    ASSERT_EQ(tokens[1].type, TokenType::Variable);
    ASSERT_EQ(tokens[1].text, "a");

    ASSERT_EQ(tokens[5].type, TokenType::PhpCloseTag);
    ASSERT_EQ(tokens[5].text, "?>");
}

TEST_CASE("Lexer: Source Location Accuracy") {
    std::string source =
        "<?php\n"
        "$a = 10;\n"
        "  $b = 20;";

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // line 1: <?php
    ASSERT_EQ(tokens[0].location.line, 1u);
    ASSERT_EQ(tokens[0].location.column, 1u);

    // line 2: $a
    ASSERT_EQ(tokens[1].location.line, 2u);
    ASSERT_EQ(tokens[1].location.column, 1u);

    // line 3: $b at col 3
    ASSERT_EQ(tokens[5].location.line, 3u);
    ASSERT_EQ(tokens[5].location.column, 3u);
}

TEST_CASE("Lexer: Invalid Tokens Handling") {
    Lexer lexer("$ 123");
    std::vector<Token> tokens = lexer.tokenize();

    ASSERT_EQ(tokens[0].type, TokenType::Invalid);
    ASSERT_EQ(tokens[0].text, "$");
    ASSERT_EQ(tokens[1].type, TokenType::IntLiteral);
    ASSERT_EQ(tokens[1].text, "123");
}
