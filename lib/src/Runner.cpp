#include <memory>
#include <vector>
#include <iomanip>

#include "Runner.h"
#include "RunValues.h"

using std::cout;
using std::endl;
using std::runtime_error;
using std::string;
using std::vector;
using std::to_string;

Runner::Runner(RootNode& root) : rootNode(root) {
    cout << endl << endl << "RUNNER_STARTS" << endl;
}

bool isNumber(const Value& val) {
    return mpark::holds_alternative<NumberValue>(val);
}

NumberValue asNumberValue(const Value& val) {
    if (const NumberValue* valDec = mpark::get_if<NumberValue>(&val)) {
        return NumberValue {valDec->mantissa, valDec->exponent};
    } else {
        throw runtime_error("Runner > asFloatType error (must never happen): this value can\'t be float");
    }
}

Value BaseNode::runNode(VariableScope& varScope) {
    throw runtime_error("BaseNode::runNode error: BaseNode is not runnable");
}

Value ExpressionNode::runNode(VariableScope& varScope) {
    throw runtime_error("ExpressionNode::runNode error: ExpressionNode is not runnable");
}
 
Value NumberNode::runNode(VariableScope& varScope) {
    // return NumberValue { this->getMantissa(), this->getExponent() };
    std::pair<long long, long> mantAndExp = this->getMantissaAndExponent();
    return NumberValue { mantAndExp.first, mantAndExp.second };
}

Value VariableNode::runNode(VariableScope& varScope) {
    string lit = this->varToken.literal;
    const Variable *var = varScope.getByName(lit, varToken);
    return var->value;
}

bool checkOperator(string operName, vector<TokenType> types) {
    for (const TokenType& type : types) {
        if (type.name == operName) return true;
    }
    return false;
}

Value BinNode::runNode(VariableScope& varScope) {
    TokenType oper = this->operToken.type;
    if (checkOperator(oper.name, {tokenTypes.PLUS(), tokenTypes.MINUS(), tokenTypes.MULT(), tokenTypes.DIVIDE()})) {
        Value left = this->left->runNode(varScope);
        Value right = this->right->runNode(varScope);
        if (oper.name == tokenTypes.PLUS().name) {
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) + asNumberValue(right);
        
        } else if (oper.name == tokenTypes.MINUS().name) {
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) - asNumberValue(right);
        
        } else if (oper.name == tokenTypes.MULT().name) {
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) * asNumberValue(right);
        
        } else if (oper.name == tokenTypes.DIVIDE().name) {
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) / asNumberValue(right);
        }
    } else if (oper.name == tokenTypes.EQUALS().name) {
        ExpressionNode* leftNode = this->left.get();
        if (VariableNode* varNode = dynamic_cast<VariableNode*>(leftNode)) { // a = ...
            std::string varName = varNode->varToken.literal;
            Value varValue = this->right->runNode(varScope);
            varScope.addVar(Variable {varName, varValue});
            return NumberValue {0};
        }
        // if (FunctionCallNode* funcDeclNode = dynamic_cast<FunctionCallNode*>(leftNode)) {  // f(...) = ...
        //     Value rawVal = funcDeclNode->callInitiator->runNode(varScope);  // Противоречие, что создать функцию f(x), нужно объявить runNode у f(x)
        // }
    }
    throw RunnerException("BinNode::runNode error: bin operator error at: {pos}", operToken);
}

Value SideEffectFuncNode::runNode(VariableScope& varScope) {
    if (this->operToken.type.name == tokenTypes.PRINT().name) {
        cout << std::setprecision(20) << this->arg->runNode(varScope) << endl;
    }
    if (this->operToken.type.name == tokenTypes.TEST().name) {
        Value val1 = this->arg->runNode(varScope);
        Value val2 = this->arg2->runNode(varScope);
        cout << ((val1 == val2) ? "true" : "false") << ", ";
        cout << val1 << ((val1 == val2) ? " == " : " != ") << val2 << endl;
        // cout << ((val1 == val2) ? "true" : "false") << endl;
        // cout << "v1 " << val1 << endl;
        // cout << "v2 " << val2 << endl;
    }
    return NumberValue {0};
}

Value FunctionStatementNode::runNode(VariableScope& varScope) {
    std::string funcName = functionName.literal;
    vector<VariableNode*> argPtrs = {};
    for (const auto& arg : this->args)
        argPtrs.push_back(arg.get());
    FunctionValue funcVal = FunctionValue {funcName, this->body.get(), argPtrs};
    // varScope.addVar(Variable {funcName, funcVal});
    varScope.addFunction(funcVal);
    return NumberValue {0};
}

Value FunctionCallNode::runNode(VariableScope& varScope) {
    string callFunctionName = this->callInitiator->varToken.literal;
    const FunctionValue* functionValue = varScope.getFunctionByName(callFunctionName, this->leftParToken);
    if (functionValue->args.size() != this->args.size()) {
        // cout << "ERR " << this->leftParToken.literal << " " << callFunctionName << " " << functionValue->args.size() << " " << this->args.size() << endl;
        // throw RunnerException("FunctionCallNode::runNode error: function \"{0}\" must be called with {1} arguments but {2} was given at: {pos}", this->leftParToken, 3,
        //     callFunctionName, (functionValue->args.size()), (this->args.size()));
        // throw RunnerException("rerr");
        // throw std::exception();
        throw RunnerException("FunctionCallNode::runNode error: function \"{0}\" must be called with {1} arguments but {2} was given at: {pos}", this->leftParToken,
            callFunctionName, (functionValue->args.size()), (this->args.size()));
    }
    
    VariableScope localScope = varScope;  // Копирование, после завершения FunctionCallNode::runNode localScope автоматически удалится
    // На примере: f(x) = x^2; f(5+2)
    for (int i = 0; i < this->args.size(); i++) {
        ExpressionNode* givenArg = this->args[i].get();  // Указатель на ExpressionNode("5+2")
        Value givenResult = givenArg->runNode(varScope);  // Считаем результат (5+2=7), тогда givenResult это NumberValue(7)

        VariableNode* requiredArg = functionValue->args[i];  // Указатель на VariableNode("x")
        string requiredArgName = requiredArg->varToken.literal;  // Получаем имя "x"

        localScope.addVar(Variable {requiredArgName, givenResult});  // Склеиваем требуемый параметр и полученное к нему значение (x = 7)
    }
    Value result = functionValue->body->runNode(localScope);
    return result;
}

Value RootNode::runNode(VariableScope& varScope) {
    for (const auto& instr : this->instructions) {
        Value result = instr->runNode(varScope);
        if (instr == instructions.back())
            cout << std::setprecision(20) << result << endl;
    }
    return NumberValue {0};
}

VariableScope Runner::run() {
    rootNode.runNode(varScope);
    return varScope;
}