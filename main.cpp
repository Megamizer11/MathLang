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

int main() {
    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);
    Runner runner = Runner(parser.rootNode);
    runner.run();
    // // std::pair<long long, long> mantAndExp = getMantissaAndExponentFromLiteral("2350.030");
    // // std::pair<long long, long> mantAndExp = getMantissaAndExponentFromLiteral("0");
    // // std::pair<long long, long> mantAndExp = getMantissaAndExponentFromLiteral("1.0");
    // // std::pair<long long, long> mantAndExp = getMantissaAndExponentFromLiteral("-10.01");
    // std::pair<long long, long> mantAndExp = getMantissaAndExponentFromLiteral("10.0");
    // std::string lit = getLiteralFromMantissaAndExponent(mantAndExp.first, mantAndExp.second);
    // // cout << "lit->\"" << lit << "\" " << mantAndExp.first << " " << mantAndExp.second << " " << mantAndExp.first * pow(10, mantAndExp.second) << endl;
    // cout << "lit->\"" << lit << "\" " << mantAndExp.first << " " << mantAndExp.second << " " << endl;
    return 0;
}