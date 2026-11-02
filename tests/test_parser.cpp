#include "test_framework.h"
#include "Lexer.h"
#include "Parser.h"
#include <sstream>

TEST_CASE("Parser: Variable Assignment and Sigil Stripping") {
    std::string code = "$total = 100;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    ASSERT_TRUE(ast != nullptr);
    ASSERT_EQ(ast->statements.size(), 1u);

    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    ASSERT_TRUE(exprStmt != nullptr);

    auto* assignExpr = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());
    ASSERT_TRUE(assignExpr != nullptr);
    ASSERT_EQ(assignExpr->varName, "total");

    auto* intVal = dynamic_cast<IntExprAST*>(assignExpr->value.get());
    ASSERT_TRUE(intVal != nullptr);
    ASSERT_EQ(intVal->value, 100);
}

TEST_CASE("Parser: Chained Variable Assignment") {
    std::string code = "$a = $b = 42;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    ASSERT_TRUE(exprStmt != nullptr);

    auto* assignA = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());
    ASSERT_TRUE(assignA != nullptr);
    ASSERT_EQ(assignA->varName, "a");

    auto* assignB = dynamic_cast<AssignExprAST*>(assignA->value.get());
    ASSERT_TRUE(assignB != nullptr);
    ASSERT_EQ(assignB->varName, "b");

    auto* intVal = dynamic_cast<IntExprAST*>(assignB->value.get());
    ASSERT_TRUE(intVal != nullptr);
    ASSERT_EQ(intVal->value, 42);
}

TEST_CASE("Parser: Operator Precedence Arithmetic") {
    // 2 + 3 * 4 should be parsed as 2 + (3 * 4)
    std::string code = "$res = 2 + 3 * 4;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    auto* assign = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());
    auto* addBin = dynamic_cast<BinaryExprAST*>(assign->value.get());

    ASSERT_TRUE(addBin != nullptr);
    ASSERT_EQ(addBin->op, "+");

    auto* lhsInt = dynamic_cast<IntExprAST*>(addBin->lhs.get());
    ASSERT_TRUE(lhsInt != nullptr);
    ASSERT_EQ(lhsInt->value, 2);

    auto* mulBin = dynamic_cast<BinaryExprAST*>(addBin->rhs.get());
    ASSERT_TRUE(mulBin != nullptr);
    ASSERT_EQ(mulBin->op, "*");
}

TEST_CASE("Parser: Parentheses Overriding Precedence") {
    // (2 + 3) * 4 should be parsed as (2 + 3) * 4
    std::string code = "$res = (2 + 3) * 4;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    auto* assign = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());
    auto* mulBin = dynamic_cast<BinaryExprAST*>(assign->value.get());

    ASSERT_TRUE(mulBin != nullptr);
    ASSERT_EQ(mulBin->op, "*");

    auto* addBin = dynamic_cast<BinaryExprAST*>(mulBin->lhs.get());
    ASSERT_TRUE(addBin != nullptr);
    ASSERT_EQ(addBin->op, "+");

    auto* rhsInt = dynamic_cast<IntExprAST*>(mulBin->rhs.get());
    ASSERT_TRUE(rhsInt != nullptr);
    ASSERT_EQ(rhsInt->value, 4);
}

TEST_CASE("Parser: Left Associativity of Subtraction") {
    // 10 - 4 - 2 should be parsed as (10 - 4) - 2
    std::string code = "$res = 10 - 4 - 2;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    auto* assign = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());
    auto* outerMinus = dynamic_cast<BinaryExprAST*>(assign->value.get());

    ASSERT_TRUE(outerMinus != nullptr);
    ASSERT_EQ(outerMinus->op, "-");

    auto* innerMinus = dynamic_cast<BinaryExprAST*>(outerMinus->lhs.get());
    ASSERT_TRUE(innerMinus != nullptr);
    ASSERT_EQ(innerMinus->op, "-");

    auto* rhsInt = dynamic_cast<IntExprAST*>(outerMinus->rhs.get());
    ASSERT_TRUE(rhsInt != nullptr);
    ASSERT_EQ(rhsInt->value, 2);
}

TEST_CASE("Parser: Logical and Unary Expressions") {
    std::string code = "$res = !$flag && $ready || $active;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    auto* assign = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());

    // Root of expression is ||
    auto* orBin = dynamic_cast<BinaryExprAST*>(assign->value.get());
    ASSERT_TRUE(orBin != nullptr);
    ASSERT_EQ(orBin->op, "||");

    // LHS is &&
    auto* andBin = dynamic_cast<BinaryExprAST*>(orBin->lhs.get());
    ASSERT_TRUE(andBin != nullptr);
    ASSERT_EQ(andBin->op, "&&");

    // LHS of && is !
    auto* notUnary = dynamic_cast<UnaryExprAST*>(andBin->lhs.get());
    ASSERT_TRUE(notUnary != nullptr);
    ASSERT_EQ(notUnary->op, "!");

    auto* flagVar = dynamic_cast<VariableExprAST*>(notUnary->operand.get());
    ASSERT_TRUE(flagVar != nullptr);
    ASSERT_EQ(flagVar->name, "flag");
}

TEST_CASE("Parser: Echo Statement with Multiple Arguments") {
    std::string code = "echo \"Count: \", $count, \"\\n\";";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    ASSERT_EQ(ast->statements.size(), 1u);

    auto* echoStmt = dynamic_cast<EchoStmtAST*>(ast->statements[0].get());
    ASSERT_TRUE(echoStmt != nullptr);
    ASSERT_EQ(echoStmt->expressions.size(), 3u);

    auto* str1 = dynamic_cast<StringExprAST*>(echoStmt->expressions[0].get());
    ASSERT_TRUE(str1 != nullptr);
    ASSERT_EQ(str1->value, "Count: ");

    auto* var2 = dynamic_cast<VariableExprAST*>(echoStmt->expressions[1].get());
    ASSERT_TRUE(var2 != nullptr);
    ASSERT_EQ(var2->name, "count");
}

TEST_CASE("Parser: If-Else Statement") {
    std::string code =
        "if ($x > 0) {\n"
        "    $status = \"positive\";\n"
        "} else {\n"
        "    $status = \"negative\";\n"
        "}";

    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* ifStmt = dynamic_cast<IfStmtAST*>(ast->statements[0].get());
    ASSERT_TRUE(ifStmt != nullptr);

    auto* cond = dynamic_cast<BinaryExprAST*>(ifStmt->condition.get());
    ASSERT_TRUE(cond != nullptr);
    ASSERT_EQ(cond->op, ">");

    auto* thenBlock = dynamic_cast<BlockStmtAST*>(ifStmt->thenBranch.get());
    ASSERT_TRUE(thenBlock != nullptr);
    ASSERT_EQ(thenBlock->statements.size(), 1u);

    auto* elseBlock = dynamic_cast<BlockStmtAST*>(ifStmt->elseBranch.get());
    ASSERT_TRUE(elseBlock != nullptr);
    ASSERT_EQ(elseBlock->statements.size(), 1u);
}

TEST_CASE("Parser: While Loop Statement") {
    std::string code =
        "while ($i < 10) {\n"
        "    $i = $i + 1;\n"
        "}";

    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* whileStmt = dynamic_cast<WhileStmtAST*>(ast->statements[0].get());
    ASSERT_TRUE(whileStmt != nullptr);

    auto* cond = dynamic_cast<BinaryExprAST*>(whileStmt->condition.get());
    ASSERT_TRUE(cond != nullptr);
    ASSERT_EQ(cond->op, "<");

    auto* bodyBlock = dynamic_cast<BlockStmtAST*>(whileStmt->body.get());
    ASSERT_TRUE(bodyBlock != nullptr);
    ASSERT_EQ(bodyBlock->statements.size(), 1u);
}

TEST_CASE("Parser: Return Statement with and without Value") {
    std::string code = "return 100; return;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    ASSERT_EQ(ast->statements.size(), 2u);

    auto* retVal = dynamic_cast<ReturnStmtAST*>(ast->statements[0].get());
    ASSERT_TRUE(retVal != nullptr);
    ASSERT_TRUE(retVal->expr != nullptr);

    auto* retVoid = dynamic_cast<ReturnStmtAST*>(ast->statements[1].get());
    ASSERT_TRUE(retVoid != nullptr);
    ASSERT_TRUE(retVoid->expr == nullptr);
}

TEST_CASE("Parser: Function Declaration and Parameters (No Sigils)") {
    std::string code =
        "function multiply($x, $y) {\n"
        "    return $x * $y;\n"
        "}";

    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    ASSERT_EQ(ast->statements.size(), 1u);

    auto* funcDecl = dynamic_cast<FunctionDeclAST*>(ast->statements[0].get());
    ASSERT_TRUE(funcDecl != nullptr);
    ASSERT_EQ(funcDecl->name, "multiply");
    ASSERT_EQ(funcDecl->params.size(), 2u);
    ASSERT_EQ(funcDecl->params[0], "x");
    ASSERT_EQ(funcDecl->params[1], "y");

    ASSERT_TRUE(funcDecl->body != nullptr);
    ASSERT_EQ(funcDecl->body->statements.size(), 1u);
}

TEST_CASE("Parser: Function Call Invocation") {
    std::string code = "$res = add(10, 20);";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_FALSE(parser.hasErrors());
    auto* exprStmt = dynamic_cast<ExprStmtAST*>(ast->statements[0].get());
    auto* assign = dynamic_cast<AssignExprAST*>(exprStmt->expr.get());
    auto* call = dynamic_cast<CallExprAST*>(assign->value.get());

    ASSERT_TRUE(call != nullptr);
    ASSERT_EQ(call->callee, "add");
    ASSERT_EQ(call->args.size(), 2u);
}

TEST_CASE("Parser: AST Dump Output Serialization") {
    std::string code = "$a = 10; echo $a;";
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    std::ostringstream ss;
    ast->dump(0, ss);
    std::string output = ss.str();

    // Verify AST dump contains the expected hierarchical tokens
    ASSERT_TRUE(output.find("ProgramAST") != std::string::npos);
    ASSERT_TRUE(output.find("AssignExpr (a =):") != std::string::npos);
    ASSERT_TRUE(output.find("IntLiteral: 10") != std::string::npos);
    ASSERT_TRUE(output.find("EchoStatement:") != std::string::npos);
    ASSERT_TRUE(output.find("Variable: a") != std::string::npos);
}

TEST_CASE("Parser: Error Diagnostics and Recovery") {
    // Missing semicolon on line 2, malformed condition on line 3
    std::string code =
        "<?php\n"
        "$a = 10\n"
        "$b = 20;\n"
        "if ($b > ) {\n"
        "    echo $b;\n"
        "}";

    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    ASSERT_TRUE(parser.hasErrors());
    const auto& errors = parser.getErrors();
    ASSERT_TRUE(errors.size() >= 2u);

    // Verify error location is captured accurately
    ASSERT_EQ(errors[0].location.line, 3u);
    ASSERT_TRUE(errors[0].message.find("Expected ';'") != std::string::npos);
}
