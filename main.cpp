#include <iostream>

#include <string>
#include <fstream>
#include <sstream>
// #include <iomanip>

#include "Lexer.h"
#include "Parser.h"
#include "Runner.h"

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

// class A { public: virtual ~A() = default; virtual void pr() { std::cout << "A" << std::endl; } };

// class B : public A { public: void pr() override { std::cout << "B" << std::endl; } };

// void pr(A* a) { std::cout << "A" << std::endl; }

// void pr(B* b) { std::cout << "B" << std::endl; }

int main() {
    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);
    Runner runner = Runner(parser.rootNode);
    runner.run();
    // A* b = new B();
    // b->pr();  // B
    // pr(b);    // A
    // delete b;
    // cout << std::setprecision(27) << 0.1 << endl;
    return 0;
}