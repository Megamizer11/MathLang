#include <iostream>

#include <string>
#include <fstream>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"
#include "Runner.h"
#include "utils.h"
#include "SymbolicComputation.h"

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

// Код очень сильно загрязнился и в нём, скорее всего, появилось много ошибок. Его нужно будет почистить и сделать код ревью
int main() {
    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);
    Runner runner = Runner(parser.rootNode);
    runner.run();

    // auto i1 = symcomp::Number {4, 0};
    // auto i2 = symcomp::Number {5, 0};
    // auto i3 = symcomp::Number {6, 0};
    // auto add = symcomp::Add<>::get(i1, i2);
    // auto add2 = symcomp::Add<>::get(add, i3);

    // auto answer = symcomp::getSafeAnswer(add2);
    // NumberValue externalAnswer = NumberValue(answer);
    // auto forcedAnswer = externalAnswer.inner->forcedCalc();
    // cout << forcedAnswer.mantissa << " " << forcedAnswer.exponent << endl;
    // cout << "ok" << endl;

    NumberValue num1 = NumberValue(symcomp::NumberWrapper {4, 0});
    NumberValue num2 = NumberValue(symcomp::NumberWrapper {5, 0});
    NumberValue answer = num1 + num2;
    auto forcedAnswer = answer.inner->forcedCalc();
    cout << forcedAnswer.mantissa << " " << forcedAnswer.exponent << endl;
    cout << "ok" << endl;
    return 0;
}