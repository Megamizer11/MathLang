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

    NumberValue num1 = NumberValue(symcomp::Log(symcomp::Number {5, 0}));
    NumberValue num2 = NumberValue(symcomp::Log(symcomp::Number {2}));
    // NumberValue num3 = NumberValue(symcomp::NumberWrapper {9, 0});
    NumberValue answer = num1 + num2;
    auto forcedAnswer = answer.inner->forcedCalc();
    std::string strAnswer = NumberValue::getAsString(forcedAnswer);
    cout << "SUM OF LOGS: LOG(5) + LOG(2) = " << strAnswer << " (" << forcedAnswer.mantissa << " " << forcedAnswer.exponent << ")" << endl;
    cout << "ok" << endl;
    return 0;
}