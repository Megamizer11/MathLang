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
        throw runtime_error("BO_Runner > asFloatType error (must never happen): this value can\'t be float");
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
            return varValue;
        }
        // if (FunctionCallNode* funcDeclNode = dynamic_cast<FunctionCallNode*>(leftNode)) {  // f(...) = ...
        //     Value rawVal = funcDeclNode->callInitiator->runNode(varScope);  // Противоречие, что создать функцию f(x), нужно объявить runNode у f(x)
        // }
    }
    throw RunnerException("BO_Runner::runNode error: bin operator error at: {pos}", operToken);
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
    return NumberValue {0};
}

Value FunctionCallNode::runNode(VariableScope& varScope) {
    // Value rawFuncValue = varScope.getByName(this->)
    return NumberValue {0};
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