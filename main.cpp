#include <iostream>

#include <string>
#include <fstream>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"

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

int main() {
    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);
    // A* b = new B();
    // b->pr();
    // delete b;
    return 0;
}