#include <iostream>

#include <string>
#include <fstream>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"
#include "Runner.h"
#include "utils.h"

using std::ifstream;
using std::stringstream;
using std::string;
using std::cout;
using std::endl;

string readFromFile(std::string name) {
    ifstream t;
    t.open(name);
    stringstream buffer;
    buffer << t.rdbuf();
    string str = buffer.str();
    t.close();
    return str;
}

// В теории такой синтаксис можно реализовать
// ∞
// ⌠
// ⌡ ƒ(x)δx
// -∞
//     ∞
// a = Σ 1/n!
//    n=0

// Grammer newGrammerRule = Grammer()
// newGrammerRule.next(NonTerminal::EXPR).next(Terminal::EXCLAM)  // Грамматика для факториала

// GrammerRule stmt = GrammerRule()
// stmt.next() ...
// GrammerRule sumOrDif = GrammerRule()
// sumOrDif.or(
//     sumOrDif.next(term),
//     sumOrDif.next(term).next(Terminal::PLUS).next(term),
//     sumOrDif.next(term).next(Terminal::MINUS).next(term)
// )
// Grammer parenthesis = Grammer().match(Terminal::OPEN_PAR).require(expr).require(Terminal::CLOSE_PAR)

// Код очень сильно загрязнился и в нём, скорее всего, появилось много ошибок. Его нужно будет почистить и сделать код ревью
int main() {
    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);
    Runner runner = Runner(parser.rootNode);
    runner.run();
    return 0;
}