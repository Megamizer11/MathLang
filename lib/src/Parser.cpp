#include <iostream>
#include <cstdarg>

#include "exceptions.h"
#include "AST_print_utils.h"
#include "Parser.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::to_string;
using std::unique_ptr;
using std::make_unique;

Parser::Parser(vector<Token> tokenList, bool showParserWork) {
    cout << endl << "PARSER_STARTS" << endl;
    this->tokenList = tokenList;
    this->pos = 0;
    this->nestedLvl = 0;

    parseCode();
    cout << "global_nodes_count: " << this->rootNode.instructions.size() << endl;
    if (showParserWork)
        this->rootNode.print();
}

bool Parser::isNext(TokenType expected) {
    if (pos < tokenList.size()) {
        if (tokenList[pos].type.name == expected.name) {
            return true;
        }
    }
    return false;
}

Token* Parser::match(TokenType expected) {
    if (pos < tokenList.size()) {
        Token *currentToken = &tokenList[pos];
        if (currentToken->type.name == expected.name) {
            pos += 1;
            return currentToken;
        }
    }
    return nullptr;
}

Token* Parser::match(vector<TokenType> expectedTokens) {
    if (pos < tokenList.size()) {
        Token *currentToken = &tokenList[pos];
        for (TokenType& expectedToken : expectedTokens) {
            if (currentToken->type.name == expectedToken.name) {
                pos += 1;
                return currentToken;
            }
        }
    }
    return nullptr;
}

Token* Parser::require(TokenType expected) {
    if (pos < tokenList.size()) {
        Token *requiredToken = match(expected);  // requiredToken возможно nullptr
        if ((requiredToken != nullptr) && (requiredToken->type.name == expected.name)) {
            return requiredToken;
        }
        throw ParserException("Parser::require error: required: \"{0}\" got: {got_literal} on pos: {pos}", tokenList[pos], 1, expected.literal);
    }
    // throw ParserException("Parser::require error: reached token limit, expected \"{0}\", got: {got_literal} at: {pos} - " + to_string(pos), tokenList[pos], 1, expected.name);
    // throw ParserException("Parser::require error: reached token limit, expected \"{0}\", got: {got_literal} at: {pos}", tokenList.back(), 1, expected.name);
    throw ParserException("Parser::require error: reached token limit, expected \"{0}\", at: {1}:{2}", tokenList.back(), 3,
            expected.name, to_string(tokenList.back().fLine), to_string(tokenList.back().fIndex + tokenList.back().literal.length()));
}

unique_ptr<VariableNode> Parser::parseVariable() {
    Token *variableToken = match(tokenTypes.IDENTIFIER());  // или через require
    if (variableToken == 0)
        throw ParserException("Parser::parseVariableDeclaration error: variable was expected at: {pos}", tokenList[pos]);
    return make_unique<VariableNode>(*variableToken);
}

unique_ptr<ExpressionNode> Parser::parsePrimary() {  // аналогично parsePrimaryFormulaUnit, парсит наименьшие части формулы: переменные и числа
    Token *variableToken = match(tokenTypes.IDENTIFIER());
    if (variableToken != 0) {
        return make_unique<VariableNode>(*variableToken);
    }
    Token *numberToken = match(tokenTypes.NUMBER());
    if (numberToken != 0)
        return make_unique<NumberNode>(*numberToken);
    throw ParserException("Parser::parsePrimary error: primary was expected at: {pos}, got: {got_literal}", tokenList[pos]);
}

unique_ptr<ExpressionNode> Parser::parseParentheses() {
    if (match(tokenTypes.OPEN_PAR())) {
        unique_ptr<ExpressionNode> expression = parseExpression();
        require(tokenTypes.CLOSE_PAR());
        return expression;
    }
    unique_ptr<ExpressionNode> expression = parsePrimary();
    return expression;
}

unique_ptr<ExpressionNode> Parser::parseMultDiv() {
    unique_ptr<ExpressionNode> left = parseParentheses();
    Token *oper = match({tokenTypes.MULT(), tokenTypes.DIVIDE()});
    while (oper != 0) {
        unique_ptr<ExpressionNode> right = parseParentheses();
        left = make_unique<BinNode>(*oper, move(left), move(right));
        oper = match({tokenTypes.MULT(), tokenTypes.DIVIDE()});
    }
    return left;
}

unique_ptr<ExpressionNode> Parser::parsePlusMinus() {
    unique_ptr<ExpressionNode> left = parseMultDiv();
    Token *oper = match({tokenTypes.PLUS(), tokenTypes.MINUS()});
    while (oper != 0) {
        unique_ptr<ExpressionNode> right = parseMultDiv();
        left = make_unique<BinNode>(*oper, move(left), move(right));
        oper = match({tokenTypes.PLUS(), tokenTypes.MINUS()});
    }
    return left;
}

unique_ptr<ExpressionNode> Parser::parseExpression() {
    return parsePlusMinus();
}

unique_ptr<ExpressionNode> Parser::parseStatement() {
    if (Token* variableToken = match(tokenTypes.IDENTIFIER())) {
        unique_ptr<VariableNode> varNode = make_unique<VariableNode>(*variableToken);
        if (Token* equalsToken = match(tokenTypes.EQUALS())) {
            unique_ptr<ExpressionNode> formulaNode = parseExpression();
            return make_unique<BinNode>(*equalsToken, move(varNode), move(formulaNode));  // a = 1 + 1
        }
        return move(varNode);  // a
    }
    if (Token* printToken = match(tokenTypes.PRINT())) {
        unique_ptr<ExpressionNode> arg = parseExpression();
        // vector<unique_ptr<ExpressionNode>> vec = {move(arg)};
        return make_unique<SideEffectFuncNode>(*printToken, move(arg), nullptr);
    }
    if (Token* printToken = match(tokenTypes.TEST())) {
        unique_ptr<ExpressionNode> arg = parseExpression();
        require(tokenTypes.EQUALS());
        unique_ptr<ExpressionNode> arg2 = parseExpression();
        // vector<unique_ptr<ExpressionNode>> vec = {move(arg), move(arg2)};
        return make_unique<SideEffectFuncNode>(*printToken, move(arg), move(arg2));
    }
    return parseExpression();  // 1 + 2
}

void Parser::parseCode() {
    this->rootNode = RootNode();
    while (this->pos < tokenList.size()) {
        unique_ptr<ExpressionNode> statement = parseStatement();
        this->rootNode.addNode(move(statement));
    }
}