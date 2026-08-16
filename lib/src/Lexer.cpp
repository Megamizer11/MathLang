#include <string>
#include <iostream>

#include <Lexer.h>
#include <tokens.h>
// #include <cctype>
#include <algorithm>
#include <iomanip>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::transform;
using std::runtime_error;
using std::to_string;

// using namespace std::string_literals;

Lexer::Lexer(string code, bool showLexerWork) {
    cout << endl << "LEXER_STARTS" << endl;
    this->code = code;
    this->pos = 0;
    this->fLine = 1;
    this->fIndex = 1;
    this->fillTokenTypesVector();
    this->lex_analysis();
    if (showLexerWork) {
        for (int i = 0; i < tokenList.size(); i++)
            cout << std::left << std::setw(4) << tokenList[i].literal << " " << std::left << std::setw(10) << tokenList[i].type.name << " " << tokenList[i].fLine << ":" << tokenList[i].fIndex << endl;
        cout << "token_count: " << tokenList.size() << endl;
    }
}

void Lexer::fillTokenTypesVector() {
    tokenTypesVector.reserve(tokenTypes.size());

    for (const TokenType& tokenType : tokenTypes) {
        if (tokenType.type == LexState::UNIQUE)
            this->uniques.push_back(tokenType);
    }

    transform(tokenTypes.begin(), tokenTypes.end(), std::back_inserter(tokenTypesVector),
                    [](const TokenType& tokenType) {
                        return tokenType;
                    });
}

void Lexer::push_token(const TokenType& tokenType, string literal) {
    Token token = Token(tokenType, literal, this->pos, this->fLine, this->fIndex);
    this->tokenList.push_back(token);
    this->pos += literal.length();
    this->fIndex += literal.length();
}

void Lexer::read_number() {
    char ch;
    std::string accum = "";
    // int c = -1;  // нужен чтобы минус мог быть только первым символом в литерале
    int isFraction = false;
    for (int i = this->pos; i < code.length(); i++) {
        // c++;
        ch = code[i];
        if (i == this->pos && ch == '-') {  // c == 0 => i == this->pos
            accum += ch;
            continue;
        }
        if (ch == '.') {
            if (!isFraction) {
                accum += ch;
                continue;
            }
            else throw runtime_error("Lexer: not number at: " + to_string(this->fLine) + string(":") + to_string(this->fIndex));
        }
        if (isdigit(ch)) {  // Здесь число может начинаться с нуля: 01
            accum += ch;
        } else break;
    }
    push_token(tokenTypes.NUMBER(), accum);
}

void Lexer::read_identifier() {
    char ch;
    std::string accum = "";
    for (int i = this->pos; i < code.length(); i++) {
        ch = code[i];
        if (isalnum(ch) || ch == '_') {
            accum += ch;
        } else break;
    }
    push_token(tokenTypes.IDENTIFIER(), accum);
}

void Lexer::read_unique() {  // Может парсить не только односимволные непвторяющиеся токены, но и такие токены. как + ++ += и т.д.
    char ch;
    const TokenType* curTokType = nullptr;
    std::string accum = "";
    bool wasFound = false;
    for (int i = this->pos; i < code.length(); i++) {
        ch = code[i];
        if (curTokType) {
            if (curTokType->literal.rfind(accum + ch, 0) == 0) {  // Если accum является префиксом текущего типа токена
                accum += ch;
                continue;
            }
        }
        wasFound = false;  // Тип токена перестал совпадать с текущим (curTokType), нужно искать новый
        for (const TokenType& unique : this->uniques) {
            if (unique.literal.rfind(accum + ch, 0) == 0) {  // Если accum является префиксом другого типа токена
                accum += ch;
                curTokType = &unique;
                wasFound = true;  // Нашли новый тип (при переходе с токена "+" на токен "+=" тип токена меняется (хотя "+=" не используется в этом языке))
                break;
            }
        }
        if (!wasFound) {
            if (accum.empty())  // Токен не был найден, так как текущий символ не является началом ни одного из типов токенов
                throw runtime_error("Lexer: undefined token at:" + to_string(this->fLine) + string(":") + to_string(this->fIndex));
            break;  // Строка accum является финальным токеном, текущий символ - начало нового токена
        }
    }
    push_token(*curTokType, accum);
}

void Lexer::lex_analysis() {
    char ch;
    // bool lastCharWasMinus = false;
    bool canStartNumberWithMinus = true;
    bool inComment = false;
    while (this->pos < this->code.length()) {
        ch = code[this->pos];
        
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            if (ch == '\n') {
                this->fLine++;
                this->fIndex = 0;
                inComment = false;
            }
            this->pos++;
            this->fIndex++;
            continue;
        }

        if (inComment) {
            this->pos++;
            continue;
        }

        if (isdigit(ch) || (canStartNumberWithMinus && ch == '-')) {
            read_number();
        } else if (isalnum(ch) || ch == '_') {
            read_identifier();
        } else {
            read_unique();
        }

        if (tokenList.back().type.name == tokenTypes.NUMBER().name ||  // Случай: 1-2
            tokenList.back().type.name == tokenTypes.IDENTIFIER().name ||  // Случай: a-2
            tokenList.back().type.name == tokenTypes.CLOSE_PAR().name)  // Случай: (a)-2
            canStartNumberWithMinus = false;  // Во всех этих случаях знак "-" является операцией
        else
            canStartNumberWithMinus = true;  // Во всех остальных случаях знак "-" показывает, что число отрицательное: 2 + -1, f(-1)
        
        if (tokenList.back().type.name == tokenTypes.COMMENT().name) {
            inComment = true;
            tokenList.pop_back();
        }
    }
}