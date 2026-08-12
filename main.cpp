#include <iostream>

#include "string"
#include "fstream"
#include "sstream"

#include <Lexer.h>

using std::ifstream;
using std::stringstream;
using std::string;

string readFromFile(std::string name) {
    ifstream t;
    t.open(name);
    stringstream buffer;
    buffer << t.rdbuf();
    string str = buffer.str();
    t.close();
    return str;
}

int main() {
    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    return 0;
}