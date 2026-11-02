#include "AST.h"

namespace {
    void printIndent(int indent, std::ostream& out) {
        for (int i = 0; i < indent; ++i) {
            out << "  ";
        }
    }
}

void IntExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "IntLiteral: " << value << "\n";
}

void FloatExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "FloatLiteral: " << value << "\n";
}

void StringExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "StringLiteral: \"" << value << "\"\n";
}

void BoolExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "BoolLiteral: " << (value ? "true" : "false") << "\n";
}

void NullExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "NullLiteral\n";
}

void VariableExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "Variable: " << name << "\n";
}

void BinaryExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "BinaryExpr ('" << op << "'):\n";
    if (lhs) lhs->dump(indent + 1, out);
    if (rhs) rhs->dump(indent + 1, out);
}

void UnaryExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "UnaryExpr ('" << op << "'):\n";
    if (operand) operand->dump(indent + 1, out);
}

void AssignExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "AssignExpr (" << varName << " =):\n";
    if (value) value->dump(indent + 1, out);
}

void CallExprAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "CallExpr: " << callee << "()\n";
    for (const auto& arg : args) {
        arg->dump(indent + 1, out);
    }
}

void ExprStmtAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "ExprStatement:\n";
    if (expr) expr->dump(indent + 1, out);
}

void EchoStmtAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "EchoStatement:\n";
    for (const auto& e : expressions) {
        e->dump(indent + 1, out);
    }
}

void ReturnStmtAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "ReturnStatement:\n";
    if (expr) {
        expr->dump(indent + 1, out);
    } else {
        printIndent(indent + 1, out);
        out << "(void)\n";
    }
}

void BlockStmtAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "BlockStatement {\n";
    for (const auto& stmt : statements) {
        stmt->dump(indent + 1, out);
    }
    printIndent(indent, out);
    out << "}\n";
}

void IfStmtAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "IfStatement:\n";
    printIndent(indent + 1, out);
    out << "Condition:\n";
    if (condition) condition->dump(indent + 2, out);
    printIndent(indent + 1, out);
    out << "Then:\n";
    if (thenBranch) thenBranch->dump(indent + 2, out);
    if (elseBranch) {
        printIndent(indent + 1, out);
        out << "Else:\n";
        elseBranch->dump(indent + 2, out);
    }
}

void WhileStmtAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "WhileStatement:\n";
    printIndent(indent + 1, out);
    out << "Condition:\n";
    if (condition) condition->dump(indent + 2, out);
    printIndent(indent + 1, out);
    out << "Body:\n";
    if (body) body->dump(indent + 2, out);
}

void FunctionDeclAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "FunctionDeclaration: " << name << "(";
    for (size_t i = 0; i < params.size(); ++i) {
        out << params[i];
        if (i + 1 < params.size()) out << ", ";
    }
    out << ")\n";
    if (body) body->dump(indent + 1, out);
}

void ProgramAST::dump(int indent, std::ostream& out) const {
    printIndent(indent, out);
    out << "ProgramAST (" << statements.size() << " statements/declarations):\n";
    for (const auto& stmt : statements) {
        stmt->dump(indent + 1, out);
    }
}
