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
    // По хорошему дополнять код нужно в препроцессоре, а ещё лучше в самом конце работы парсера, но что есть, то есть
    // Из-за того, что текст тупо добавляется перед кодом, это сбивает позицию при выводе ошибки
    string code = R"(
ln(n) = (#LN n)
log(n, base) = ln(n) / ln(base)
abs(n) = (#IF n, n) + (#IF -n, -n)
sign(n) = (#IF n, 1) + (#IF -n, -1)
fullif(cond, gr, ls, zer) = (#IF sign(cond), gr) + (#IF sign(-cond), ls) + (#IF -(sign(cond)^2) + 1, zer)  #COM cond>0 => gr, cond<0 => ls, cond=0 => zer

if0(n, k) = #IF n, k
if1(n, k) = (#IF n, k-1) + 1
)";
    // string code = "";
    code += readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);
    Runner runner = Runner(parser.rootNode);
    runner.run();

    // NumberValue num1 = NumberValue(symcomp::Log(symcomp::Number {5, 0}));
    // NumberValue num2 = NumberValue(symcomp::Log(symcomp::Number {2}));
    // NumberValue num3 = NumberValue(symcomp::NumberWrapper {9, 0});
    // NumberValue answer = num1 + num2;
    // NumberValue answer = NumberValue(symcomp::Exponent(
    //     std::make_shared<symcomp::NumberWrapper>(3, 7),
    //     std::make_shared<symcomp::NumberWrapper>(25, -1)
    // ));
    // auto forcedAnswer = answer.inner->forcedCalc();
    // std::string strAnswer = NumberValue::getAsString(forcedAnswer);
    // // cout << "SUM OF LOGS: LOG(5) + LOG(2) = " << strAnswer << " (" << forcedAnswer.mantissa << " " << forcedAnswer.exponent << ")" << endl;
    // cout << "50^0.2 = " << strAnswer << " (" << forcedAnswer.mantissa << " " << forcedAnswer.exponent << ")" << endl;
    return 0;
}