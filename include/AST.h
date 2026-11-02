#pragma once

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cstdint>

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void dump(int indent, std::ostream& out) const = 0;
};

// Base class for expressions
class ExprAST : public ASTNode {};

class IntExprAST : public ExprAST {
public:
    int64_t value;
    explicit IntExprAST(int64_t val) : value(val) {}
    void dump(int indent, std::ostream& out) const override;
};

class FloatExprAST : public ExprAST {
public:
    double value;
    explicit FloatExprAST(double val) : value(val) {}
    void dump(int indent, std::ostream& out) const override;
};

class StringExprAST : public ExprAST {
public:
    std::string value;
    explicit StringExprAST(std::string val) : value(std::move(val)) {}
    void dump(int indent, std::ostream& out) const override;
};

class BoolExprAST : public ExprAST {
public:
    bool value;
    explicit BoolExprAST(bool val) : value(val) {}
    void dump(int indent, std::ostream& out) const override;
};

class NullExprAST : public ExprAST {
public:
    NullExprAST() = default;
    void dump(int indent, std::ostream& out) const override;
};

class VariableExprAST : public ExprAST {
public:
    std::string name; // variable name without '$'
    explicit VariableExprAST(std::string name)
        : name(!name.empty() && name.front() == '$' ? name.substr(1) : std::move(name)) {}
    void dump(int indent, std::ostream& out) const override;
};

class BinaryExprAST : public ExprAST {
public:
    std::string op;
    std::unique_ptr<ExprAST> lhs;
    std::unique_ptr<ExprAST> rhs;

    BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
        : op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void dump(int indent, std::ostream& out) const override;
};

class UnaryExprAST : public ExprAST {
public:
    std::string op;
    std::unique_ptr<ExprAST> operand;

    UnaryExprAST(std::string op, std::unique_ptr<ExprAST> operand)
        : op(std::move(op)), operand(std::move(operand)) {}
    void dump(int indent, std::ostream& out) const override;
};

class AssignExprAST : public ExprAST {
public:
    std::string varName; // variable name without '$'
    std::unique_ptr<ExprAST> value;

    AssignExprAST(std::string varName, std::unique_ptr<ExprAST> value)
        : varName(!varName.empty() && varName.front() == '$' ? varName.substr(1) : std::move(varName)),
          value(std::move(value)) {}
    void dump(int indent, std::ostream& out) const override;
};

class CallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;

    CallExprAST(std::string callee, std::vector<std::unique_ptr<ExprAST>> args)
        : callee(std::move(callee)), args(std::move(args)) {}
    void dump(int indent, std::ostream& out) const override;
};

// Base class for statements
class StmtAST : public ASTNode {};

class ExprStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr;
    explicit ExprStmtAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)) {}
    void dump(int indent, std::ostream& out) const override;
};

class EchoStmtAST : public StmtAST {
public:
    std::vector<std::unique_ptr<ExprAST>> expressions;
    explicit EchoStmtAST(std::vector<std::unique_ptr<ExprAST>> expressions)
        : expressions(std::move(expressions)) {}
    void dump(int indent, std::ostream& out) const override;
};

class ReturnStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr; // May be nullptr
    explicit ReturnStmtAST(std::unique_ptr<ExprAST> expr = nullptr) : expr(std::move(expr)) {}
    void dump(int indent, std::ostream& out) const override;
};

class BlockStmtAST : public StmtAST {
public:
    std::vector<std::unique_ptr<StmtAST>> statements;
    BlockStmtAST() = default;
    explicit BlockStmtAST(std::vector<std::unique_ptr<StmtAST>> statements)
        : statements(std::move(statements)) {}
    void dump(int indent, std::ostream& out) const override;
};

class IfStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<StmtAST> thenBranch;
    std::unique_ptr<StmtAST> elseBranch; // May be nullptr

    IfStmtAST(std::unique_ptr<ExprAST> condition,
              std::unique_ptr<StmtAST> thenBranch,
              std::unique_ptr<StmtAST> elseBranch = nullptr)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {}
    void dump(int indent, std::ostream& out) const override;
};

class WhileStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> condition;
    std::unique_ptr<StmtAST> body;

    WhileStmtAST(std::unique_ptr<ExprAST> condition, std::unique_ptr<StmtAST> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    void dump(int indent, std::ostream& out) const override;
};

class FunctionDeclAST : public StmtAST {
public:
    std::string name;
    std::vector<std::string> params; // e.g. ["a", "b"] (without '$')
    std::unique_ptr<BlockStmtAST> body;

    FunctionDeclAST(std::string name, std::vector<std::string> params, std::unique_ptr<BlockStmtAST> body)
        : name(std::move(name)), body(std::move(body)) {
        for (auto& p : params) {
            if (!p.empty() && p.front() == '$') {
                this->params.push_back(p.substr(1));
            } else {
                this->params.push_back(std::move(p));
            }
        }
    }
    void dump(int indent, std::ostream& out) const override;
};

// Root of the AST
class ProgramAST : public ASTNode {
public:
    std::vector<std::unique_ptr<StmtAST>> statements;

    explicit ProgramAST(std::vector<std::unique_ptr<StmtAST>> statements)
        : statements(std::move(statements)) {}
    void dump(int indent, std::ostream& out) const override;
};
