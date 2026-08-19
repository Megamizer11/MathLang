#pragma once

#include <vector>
#include <memory>

#include "tokens.h"
#include "AST.h"

class Parser {
private:
    int pos;
    std::vector<Token> tokenList {};
    int nestedLvl;
    
    Token* match(TokenType expected);
    Token* match(std::vector<TokenType> expected);
    Token* require(TokenType expected);
    bool checkNext(int, TokenType);
    
    std::unique_ptr<VariableNode> parseVariable();
    std::unique_ptr<ExpressionNode> parsePrimary();
    std::unique_ptr<ExpressionNode> parseParentheses();
    std::unique_ptr<ExpressionNode> parseExponentiation();  // Возведение в степень
    std::unique_ptr<ExpressionNode> parseMultDivPercent();
    std::unique_ptr<ExpressionNode> parsePlusMinus();

    bool nextIsFunctionStatement();
    std::unique_ptr<FunctionStatementNode> parseFuncStatement();
    std::unique_ptr<FunctionCallNode> parseFuncCall();
    std::unique_ptr<SideEffectFuncNode> parseSumma();  // Для суммы пришлось создать отдельную parse функцию, т.к. она может быть частью выражения: a = (#SUM n=1, 10, n^2) * 2
    std::unique_ptr<SideEffectFuncNode> parseNaturalLog();
    
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseStatement();
    void parseCode();
public:
    RootNode rootNode;
    Parser(std::vector<Token>, bool);
};