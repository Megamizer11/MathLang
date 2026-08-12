#pragma once

#include <string>
#include <vector>

enum class LexState {
    UNIQUE,  // Все токены, которое имеют фиксированный строковый литерал, наример: + - * /
    // STRING,  // Только для токенов, которые являются строковыми литералами: "", "abc", "var = \"1\""
    NUMBER,  // Только для токенов, которые являются числами: 1, 3.14, -12, 0
    IDENTIFIER
};

class TokenType {
public:
    std::string name;
    LexState type;
    std::string literal;  // у UNIQUE токенов это будет "+", "-" и т.д., т.е. сочетание символов, которое будет использовать лексер для поиска
    // У остальных токенов это строка, которая будет указываться при ошибке (required "number" at 1:5)
    TokenType(std::string name, LexState type, std::string literal)
        : name(name), type(type), literal(literal) {}
    TokenType() {}
};

struct TokenTypes {
    std::vector<TokenType> data {
        TokenType("IDENTIFIER",  LexState::IDENTIFIER,  "identifier"),  // переменная или название функции
        TokenType("NUMBER",      LexState::NUMBER,      "number"    ),
        // TokenType("SPACE",       LexState::UNIQUE,      "space"     ),
        TokenType("EQUALS",      LexState::UNIQUE,      "="         ),
        TokenType("PLUS",        LexState::UNIQUE,      "+"         ),
        TokenType("MINUS",       LexState::UNIQUE,      "-"         ),
        TokenType("MULT",        LexState::UNIQUE,      "*"         ),
        TokenType("DIVIDE",      LexState::UNIQUE,      "/"         ),
        TokenType("POWER",       LexState::UNIQUE,      "^"         )
    };

    TokenType& IDENTIFIER() { return data[0]; }
    TokenType& NUMBER()     { return data[1]; }
    TokenType& EQUALS()     { return data[2]; }
    TokenType& PLUS()       { return data[3]; }
    TokenType& MINUS()      { return data[4]; }
    TokenType& MULT()       { return data[5]; }
    TokenType& DIVIDE()     { return data[6]; }
    TokenType& POWER()      { return data[7]; }

    std::size_t size() const {
        return data.size();
    }

    auto begin() {
        return std::begin(data);
    }

    auto end() {
        return std::end(data);
    }
};

extern TokenTypes tokenTypes;  // Если бы существовал tokens.cpp
// TokenTypes tokenTypes;

class Token {
public:
    TokenType type;
    std::string literal;
    int pos;
    int fLine;  // (f - format) На какой строке находится токен
    int fIndex;  // (f - format) Индекс символа в пределах строки
    Token(TokenType type, std::string literal, int pos, int fLine, int fIndex)
        : type(type), literal(literal), pos(pos), fLine(fLine), fIndex(fIndex) {}
    Token() {}
    // void print(int);
};