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

    auto i1 = symcomp::Number {4, 0};
    auto i2 = symcomp::Number {5, 0};
    auto i3 = symcomp::Number {5, 0};
    // auto add = symcomp::Add<decltype(i1), decltype(i2)>::get(i1, i2);
    // auto add = symcomp::Add<int, int>::get(i1, i2);
    // auto add2 = symcomp::Add<decltype(add), decltype(i2)>::get(add, i2);
    auto add = symcomp::Add<>::get(i1, i2);
    auto add2 = symcomp::Add<>::get(add, i3);
    auto answer = symcomp::getSafeAnswer(add2);
    cout << "start" << endl;
    // symcomp::SymComp result = symcomp::toUnivesal(add2);
    // cout << symcomp::toNumberValue(add2) << endl;
    // cout << symcomp::toNumberValue(add2) << endl;
    NumberValue externalAdd = NumberValue(answer);
    auto forcedAnswer = externalAdd.inner->forcedCalc();
    cout << forcedAnswer.mantissa << " " << forcedAnswer.exponent << endl;
    // cout << answer.mantissa << " " << answer.exponent << endl;
    cout << "ok" << endl;
    // cout << sizeof(symcomp::Base) << endl;
    return 0;
}