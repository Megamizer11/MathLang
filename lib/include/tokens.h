#pragma once

#include <string>
#include <vector>

enum class LexState {
    UNIQUE,  // Все токены, которое имеют фиксированный строковый литерал, наример: + - * /
    // STRING,  // Только для токенов, которые являются строковыми литералами: "", "abc", "var = \"1\""
    NUMBER,  // Только для токенов, которые являются числами: 1, 3.14, -12, 0
    IDENTIFIER,  // Обычное слово (название переменной или функции)
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
        TokenType("NUMBER",       LexState::NUMBER,      "number"    ),
        TokenType("OPEN_PAR",     LexState::UNIQUE,      "("         ),
        TokenType("CLOSE_PAR",    LexState::UNIQUE,      ")"         ),
        TokenType("COMMA",        LexState::UNIQUE,      ","         ),
        TokenType("EQUALS",       LexState::UNIQUE,      "="         ),
        TokenType("PLUS",         LexState::UNIQUE,      "+"         ),
        TokenType("MINUS",        LexState::UNIQUE,      "-"         ),
        TokenType("MULT",         LexState::UNIQUE,      "*"         ),
        TokenType("DIVIDE",       LexState::UNIQUE,      "/"         ),
        TokenType("POWER",        LexState::UNIQUE,      "^"         ),
        TokenType("PERCENT",      LexState::UNIQUE,      "%"         ),

        // Следующие типы токенов нужны для дополнительных возможностей (side эффекты, комментарии и т.д.)
        TokenType("PRINT",        LexState::UNIQUE,      "#PRINT"    ),  // Работает на уровне парсера и раннера
        TokenType("TEST",         LexState::UNIQUE,      "#TEST"     ),  // Работает на уровне парсера и раннера
        TokenType("COMMENT",      LexState::UNIQUE,      "#COM"      ),  // Работает на уровне лексера
        TokenType("SUMMA",        LexState::UNIQUE,      "#SUM"      ),  // Работает на уровне парсера (в качестве выражения) и раннера
        TokenType("NATURAL_LOG",  LexState::UNIQUE,      "#LN"       )   // Работает на уровне парсера (в качестве выражения) и раннера
    };

    TokenType& IDENTIFIER()  { return data[0];  }
    TokenType& NUMBER()      { return data[1];  }
    TokenType& OPEN_PAR()    { return data[2];  }
    TokenType& CLOSE_PAR()   { return data[3];  }
    TokenType& COMMA()       { return data[4];  }
    TokenType& EQUALS()      { return data[5];  }
    TokenType& PLUS()        { return data[6];  }
    TokenType& MINUS()       { return data[7];  }
    TokenType& MULT()        { return data[8];  }
    TokenType& DIVIDE()      { return data[9];  }
    TokenType& POWER()       { return data[10]; }
    TokenType& PERCENT()     { return data[11]; }
    TokenType& PRINT()       { return data[12]; }
    TokenType& TEST()        { return data[13]; }
    TokenType& COMMENT()     { return data[14]; }
    TokenType& SUMMA()       { return data[15]; }
    TokenType& NATURAL_LOG() { return data[16]; }

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