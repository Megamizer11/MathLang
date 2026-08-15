#pragma once

#include <string>
#include <vector>

enum class LexState {
    UNIQUE,  // Все токены, которое имеют фиксированный строковый литерал, наример: + - * /
    // STRING,  // Только для токенов, которые являются строковыми литералами: "", "abc", "var = \"1\""
    NUMBER,  // Только для токенов, которые являются числами: 1, 3.14, -12, 0
    IDENTIFIER,  // Обычное слово (название переменной или функции)
    // SIDE_EFFECT  // Название функций, начинающихся с "#" (например #PRINT), использующие побочные эффекты
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
    
    void print(int);
};

struct TokenTypes {
    std::vector<TokenType> data {
        TokenType("IDENTIFIER",   LexState::IDENTIFIER,  "identifier"),  // переменная или название функции
        // TokenType("SIDE_EFFECTS", LexState::IDENTIFIER,  "identifier"),
        TokenType("NUMBER",       LexState::NUMBER,      "number"    ),
        TokenType("OPEN_PAR",     LexState::UNIQUE,      "("         ),
        TokenType("CLOSE_PAR",    LexState::UNIQUE,      ")"         ),
        TokenType("EQUALS",       LexState::UNIQUE,      "="         ),
        TokenType("PLUS",         LexState::UNIQUE,      "+"         ),
        TokenType("MINUS",        LexState::UNIQUE,      "-"         ),
        TokenType("MULT",         LexState::UNIQUE,      "*"         ),
        TokenType("DIVIDE",       LexState::UNIQUE,      "/"         ),
        TokenType("POWER",        LexState::UNIQUE,      "^"         ),
        TokenType("PRINT",        LexState::UNIQUE,      "#PRINT"    ),
        TokenType("TEST",         LexState::UNIQUE,      "#TEST"    )
    };

    TokenType& IDENTIFIER() { return data[0];  }
    TokenType& NUMBER()     { return data[1];  }
    TokenType& OPEN_PAR()   { return data[2];  }
    TokenType& CLOSE_PAR()  { return data[3];  }
    TokenType& EQUALS()     { return data[4];  }
    TokenType& PLUS()       { return data[5];  }
    TokenType& MINUS()      { return data[6];  }
    TokenType& MULT()       { return data[7];  }
    TokenType& DIVIDE()     { return data[8];  }
    TokenType& POWER()      { return data[9];  }
    TokenType& PRINT()      { return data[10]; }
    TokenType& TEST()       { return data[11]; }

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
    std::string literal;  // Для UNIQUE токенов то же самое, что TokenType::literal, для переменных, чисел и т.д. само название ("a1" вместо "variable", "3.14" вместо "nmber")
    int pos;
    int fLine;  // (f - format) На какой строке находится токен
    int fIndex;  // (f - format) Индекс символа в пределах строки
    Token(TokenType type, std::string literal, int pos, int fLine, int fIndex)
        : type(type), literal(literal), pos(pos), fLine(fLine), fIndex(fIndex) {}
    Token() {}

    void print(int);
};