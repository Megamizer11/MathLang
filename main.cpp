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

    auto i1 = symcomp::Integer {4};
    auto i2 = symcomp::Integer {5};
    auto i3 = symcomp::Integer {5};
    auto add = symcomp::Add::get(i1, i2);
    auto add2 = symcomp::Add::get(add, i2);
    cout << add2.forcedCalc() << endl;
    cout << "ok" << endl;
    return 0;
}