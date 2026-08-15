#pragma once

#include <string>

#include "tokens.h"

class Lexer {
private:
    int pos;
    int fLine;  // (f - format) На какой строке находится токен
    int fIndex;  // (f - format) Индекс символа в пределах строки
    std::string code;
    std::vector<TokenType> tokenTypesVector;  // Список values из tokenTypes
    std::vector<TokenType> keywords {};
    std::vector<TokenType> uniques {};
    void fillTokenTypesVector();
    void push_token(const TokenType&, std::string);
    // void read_str();
    void read_number();
    void read_identifier();
    // void read_side_effect();
    void read_unique();
    void lex_analysis();
    // void format_token_list();
public:
    std::vector<Token> tokenList {};
    Lexer(std::string, bool);
};