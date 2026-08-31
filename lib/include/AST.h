#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <string>

#include "tokens.h"
// #include "RunValues.h"  // Зависимости AST от runtime быть не должно, это временное (наверное) решение
#include "utils.h"

#include "mpark/variant.hpp"

struct NumberValue;
struct FunctionValue;
class VariableScope;

using Value = mpark::variant<  // Такое решение тоже не самое хорошее
    NumberValue,
    FunctionValue
>;

class BaseNode {
public:
    virtual ~BaseNode() = default;  // нужно, чтобы работал dynamic_cast    
    virtual void print(int);
    virtual Value runNode(VariableScope&);
};

class ExpressionNode : public BaseNode {  // выражение, которое возвращает значение (например 1+2)
public:
    void print(int) override;
    Value runNode(VariableScope&) override;
};

class RootNode : public ExpressionNode {  // эквивалентно StatementsNode
public:
    std::vector<std::unique_ptr<ExpressionNode>> instructions {};  // сделав инстуркции expression, а не statement нодами, будет разрешены строки по типу 1+2;
    // std::string tag = "";  // Необязательное поле, нужен при выводе ошибок

    void addNode(std::unique_ptr<ExpressionNode> node) {
        instructions.push_back(std::move(node));
    }

    RootNode() {};
    void print(int) override;
    Value runNode(VariableScope&) override;
};

class NumberNode : public ExpressionNode {
public:
    Token numberToken;
    // std::pair<long long, long> _mantAndExp = getMantissaAndExponentFromLiteral(numberToken.literal);

    NumberNode() {};

    NumberNode(Token numberToken) {
        this->numberToken = numberToken;
    };

    bool isInt() {
        return numberToken.literal.find(".") == std::string::npos;
    }

    std::pair<long long, long> getMantissaAndExponent() {
        return getMantissaAndExponentFromLiteral(numberToken.literal);
    }

    // long long getMantissa() {
    //     if (isInt())
    //         return std::stoi(numberToken.literal);
    //     std::string mantissa = numberToken.literal;
    //     mantissa.erase(numberToken.literal.find("."), 1);
    //     return std::stoi(mantissa);
    // }

    // long getExponent() {
    //     if (isInt())
    //         return 0;
    //     return - ((numberToken.literal.length() - 1) - numberToken.literal.find("."));  // Если число дробное, экспонента отрицательна
    // }

    void print(int) override;
    Value runNode(VariableScope&) override;
};

class VariableNode : public ExpressionNode {
public:
    Token varToken;

    VariableNode() {};

    VariableNode(Token varToken) {
        this->varToken = varToken;
    };
    
    void print(int) override;
    Value runNode(VariableScope&) override;
};

class BinNode : public ExpressionNode {  // сделав любые bin ноды выржанениями, можно добиться синтаксиса по типу: a = b = 0
public:
    Token operToken;
    std::unique_ptr<ExpressionNode> left;
    std::unique_ptr<ExpressionNode> right;

    BinNode() {};

    BinNode(Token operToken, std::unique_ptr<ExpressionNode> left, std::unique_ptr<ExpressionNode> right) {
        this->operToken = operToken;
        this->left = std::move(left);
        this->right = std::move(right);
    };
    
    void print(int) override;
    Value runNode(VariableScope&) override;
};

class UnarNode : public ExpressionNode {
public:
    Token operToken;
    std::unique_ptr<ExpressionNode> operand;

    UnarNode() {};

    UnarNode(Token operToken, std::unique_ptr<ExpressionNode> operand) {
        this->operToken = operToken;
        this->operand = move(operand);
    };

    void print(int) override;
    Value runNode(VariableScope&) override;
};

class SideEffectFuncNode : public ExpressionNode {
public:
    Token operToken;
    // std::vector<ExpressionNode*> args;
    std::vector<std::unique_ptr<ExpressionNode>> args;

    SideEffectFuncNode() {};

    SideEffectFuncNode(Token operToken, std::vector<std::unique_ptr<ExpressionNode>> args) {
        this->operToken = operToken;
        this->args = std::move(args);
    };
    
    void print(int) override;
    Value runNode(VariableScope&) override;
};

class FunctionStatementNode : public ExpressionNode {  // f(x) = x^2 + 2
public:
    Token functionName;
    std::unique_ptr<ExpressionNode> body;
    std::vector<std::unique_ptr<VariableNode>> args;

    FunctionStatementNode() {};
    
    FunctionStatementNode(Token functionName, std::unique_ptr<ExpressionNode> body, std::vector<std::unique_ptr<VariableNode>> args) {
        this->functionName = functionName;
        this->body = move(body);
        this->args = move(args);
    };

    void print(int) override;
    Value runNode(VariableScope&) override;
};

class FunctionCallNode : public ExpressionNode {  // f(2+2)
public:
    Token leftParToken;
    // std::unique_ptr<ExpressionNode> callInitiator;
    // Убрана возможность цепочных вызовов (f(1)(2)), т.к. могла бы получиться ситуация (f(x)(y) = z), при которой нельзя понять без семантики, корректное ли выражение
    std::unique_ptr<VariableNode> callInitiator;
    std::vector<std::unique_ptr<ExpressionNode>> args;  // важно: аргумент это не VariableNode, а именно ExpressionNode

    FunctionCallNode() {};
    
    FunctionCallNode(Token leftParToken, std::unique_ptr<VariableNode> callInitiator, std::vector<std::unique_ptr<ExpressionNode>> args) {
        this->leftParToken = leftParToken;
        this->callInitiator = move(callInitiator);
        this->args = move(args);
    };

    void print(int) override;
    Value runNode(VariableScope&) override;
};