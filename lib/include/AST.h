#pragma once

#include <vector>
#include <memory>
#include <iostream>

#include "tokens.h"
#include "RunValues.h"  // Зависимости AST от runtime быть не должно, это временное (наверное) решение

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

    NumberNode() {};

    NumberNode(Token numberToken) {
        this->numberToken = numberToken;
    };

    bool isInt() {
        return numberToken.literal.find(".") == std::string::npos;
    }

    // long long getInt() {
    //     if (isInt())
    //         return std::stoi(numberToken.literal);
    //     return std::stoi(numberToken.literal.substr(0, numberToken.literal.find(".")));
    // }

    // long long getDecimal() {
    //     if (isInt())
    //         return 0;
    //     return std::stoi(numberToken.literal.substr(numberToken.literal.find(".") + 1));
    // }

    long long getMantissa() {
        if (isInt())
            return std::stoi(numberToken.literal);
        // return std::stoi(numberToken.literal.substr(0, numberToken.literal.find(".")) +
        //                  numberToken.literal.substr(numberToken.literal.find(".") + 1)
        //                 );
        std::string mantissa = numberToken.literal;
        mantissa.erase(numberToken.literal.find("."), 1);
        // std::cout << "mantissa " << mantissa << std::endl;
        return std::stoi(mantissa);
    }

    long getExponent() {
        if (isInt())
            return 0;
        return - ((numberToken.literal.length() - 1) - numberToken.literal.find("."));  // Если число дробное, экспонента отрицательна
    }

    // long getExponent() {
    //     if (isInt())
    //         return 0;
    //     // std::cout << "exp " << numberToken.literal.length() - 1 - numberToken.literal.find(".") << std::endl;
    //     return (numberToken.literal.length() - 1) - numberToken.literal.find(".");
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