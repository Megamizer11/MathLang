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

// Пролог - объявление базовых функций (таких как abs, log) в самом начале, чтобы их не приходилось объявлять в самом коде
RootNode getPrologueRootNode() {
    string prologue =
R"(
e = #E
pi = #PI
ln(n) = (#LN n)
log(n, base) = ln(n) / ln(base)
lg(n, base) = ln(n) / ln(10)
abs(n) = (#IF n, n) + (#IF -n, -n)
sign(n) = (#IF n, 1) + (#IF -n, -1)
ifex(cond, gr, ls, zer) = (#IF sign(cond), gr) + (#IF sign(-cond), ls) + (#IF -(sign(cond)^2) + 1, zer)  #COM cond>0 => gr, cond<0 => ls, cond=0 => zer

if0(n, k) = #IF n, k
if1(n, k) = (#IF n, k-1) + 1
)";
    RootNode prologueRootNode;
    try {
        Lexer prologueLexer = Lexer(prologue, true);
        Parser prologueParser = Parser(prologueLexer.tokenList, true);
        prologueRootNode = std::move(prologueParser.rootNode);
    } catch (LexerException e) {
        auto msg = e.what();
        throw LexerException("PROLOGUE COMPILE UNEXPECTED ERROR: " + std::string(msg));
    } catch (ParserException e) {
        auto msg = e.what();
        throw ParserException("PROLOGUE COMPILE UNEXPECTED ERROR: " + std::string(msg));
    } catch (RunnerException e) {
        auto msg = e.what();
        throw RunnerException("PROLOGUE COMPILE UNEXPECTED ERROR: " + std::string(msg));
    }
    return prologueRootNode;
}

// Код очень загрязнился и в нём, скорее всего, появилось много ошибок. Его нужно будет почистить и сделать код ревью
int main() {
    RootNode prologueRootNode = getPrologueRootNode();
    auto& prologueAST = prologueRootNode.instructions;

    string code = readFromFile("code.mthl");
    Lexer lexer = Lexer(code, true);
    Parser parser = Parser(lexer.tokenList, true);

    parser.rootNode.instructions.insert(
        parser.rootNode.instructions.begin(),
        std::make_move_iterator(prologueAST.begin()),
        std::make_move_iterator(prologueAST.end())
    );

    Runner runner = Runner(parser.rootNode);
    runner.run();
    return 0;
}