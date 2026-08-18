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

bool Parser::checkNext(int lookahead, TokenType expected) {  // checkNext(0, ...) провряет текущий символ, checkNext(1, ...) провряет следующий символ, checkNext(2, ...) послеследующий символ
    if (pos + lookahead < tokenList.size()) {
        if (tokenList[pos + lookahead].type.name == expected.name) {
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
    throw ParserException("Parser::require error: reached token limit, expected \"{0}\", at: {1}:{2}", tokenList.back(), 3,
            expected.name, to_string(tokenList.back().fLine), to_string(tokenList.back().fIndex + tokenList.back().literal.length()));
}

unique_ptr<VariableNode> Parser::parseVariable() {
    Token *variableToken = match(tokenTypes.IDENTIFIER());  // или через require
    if (variableToken == 0)
        throw ParserException("Parser::parseVariable error: variable was expected at: {pos}", tokenList[pos]);
    return make_unique<VariableNode>(*variableToken);
}

unique_ptr<ExpressionNode> Parser::parsePrimary() {  // аналогично parsePrimaryFormulaUnit, парсит наименьшие части формулы: переменные и 
    if (checkNext(0, tokenTypes.IDENTIFIER()), checkNext(1, tokenTypes.OPEN_PAR())) {
        return parseFuncCall();
    }
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

bool Parser::nextIsFunctionStatement() {
    if (checkNext(0, tokenTypes.IDENTIFIER()) && checkNext(1, tokenTypes.OPEN_PAR())) {
        // if (checkNext(1, tokenTypes.IDENTIFIER()))
        int i = 2;
        while (!checkNext(i, tokenTypes.CLOSE_PAR())) {  // Находим тот i, на котором будет CLOSE_PAR
            i++;
            // if (pos + i > tokenList.size()) goto err;
            if (pos + i > tokenList.size()) break;
        }
        return checkNext(i+1, tokenTypes.EQUALS());  // Даже если дойдёт до конца файла (произойдёт break), functionIsStatement вернёт false
    }
    throw ParserException("Not a function at: {pos}", tokenList[pos]);
}

unique_ptr<FunctionStatementNode> Parser::parseFuncStatement() {
    Token *variableToken = match(tokenTypes.IDENTIFIER());
    vector<unique_ptr<VariableNode>> args = {};
    require(tokenTypes.OPEN_PAR());
    while (!checkNext(0, tokenTypes.CLOSE_PAR())) {
        unique_ptr<VariableNode> arg = parseVariable();
        args.push_back(move(arg));
        if (!checkNext(0, tokenTypes.CLOSE_PAR()))
            require(tokenTypes.COMMA());
    }
    require(tokenTypes.CLOSE_PAR());
    require(tokenTypes.EQUALS());
    unique_ptr<ExpressionNode> body = parseExpression();
    return make_unique<FunctionStatementNode>(*variableToken, move(body), move(args));
}

unique_ptr<FunctionCallNode> Parser::parseFuncCall() {
    unique_ptr<VariableNode> left = parseVariable();
    vector<unique_ptr<ExpressionNode>> args = {};
    Token* openPar = require(tokenTypes.OPEN_PAR());
    while (!checkNext(0, tokenTypes.CLOSE_PAR())) {
        unique_ptr<ExpressionNode> arg = parseExpression();
        args.push_back(move(arg));
        if (!checkNext(0, tokenTypes.CLOSE_PAR()))
            require(tokenTypes.COMMA());
    }
    require(tokenTypes.CLOSE_PAR());
    return make_unique<FunctionCallNode>(*openPar, move(left), move(args));
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
    if (checkNext(0, tokenTypes.IDENTIFIER())) {  // a
        if (nextIsFunctionStatement())  // Мы точно знаем, что следующее сочетание токенов - объявление функция
            return parseFuncStatement();
        
        if (checkNext(1, tokenTypes.OPEN_PAR()))  // Здесь мы будем парсить вызов функции
            return parseFuncCall();
        
        // Мы знаем, что будем парсить НЕ функцию. Предположительно "a =" или просто "a"
        unique_ptr<VariableNode> varNode = parseVariable();
        if (Token* equalsToken = match(tokenTypes.EQUALS())) {
            unique_ptr<ExpressionNode> formulaNode = parseExpression();
            return make_unique<BinNode>(*equalsToken, move(varNode), move(formulaNode));  // a = 1 + 1
        }
        return move(varNode);
    }
    if (Token* printToken = match(tokenTypes.PRINT())) {
        unique_ptr<ExpressionNode> arg = parseExpression();
        return make_unique<SideEffectFuncNode>(*printToken, move(arg), nullptr);
    }
    if (Token* printToken = match(tokenTypes.TEST())) {
        unique_ptr<ExpressionNode> arg = parseExpression();
        require(tokenTypes.EQUALS());
        unique_ptr<ExpressionNode> arg2 = parseExpression();
        return make_unique<SideEffectFuncNode>(*printToken, move(arg), move(arg2));
    }
    return parseExpression();  // 1 + 2  // Из-за return move(left); (выше в этой функции) эта строка парсит выражения (как отдельные строки) по типу 1 + d, а просто d парсит return move(left);
}

void Parser::parseCode() {
    this->rootNode = RootNode();
    while (this->pos < tokenList.size()) {
        unique_ptr<ExpressionNode> statement = parseStatement();
        this->rootNode.addNode(move(statement));
    }
}