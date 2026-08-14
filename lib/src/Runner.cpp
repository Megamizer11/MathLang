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
    return NumberValue { this->getMantissa(), this->getExponent() };
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
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) + asNumberValue(right);
        
        } else if (oper.name == tokenTypes.MULT().name) {
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) * asNumberValue(right);
        
        } else if (oper.name == tokenTypes.DIVIDE().name) {
            if (isNumber(left) && isNumber(right)) return asNumberValue(left) / asNumberValue(right);
        }
    } else if (oper.name == tokenTypes.EQUALS().name) {
        ExpressionNode* leftNode = this->left.get();
        if (VariableNode* varNode = dynamic_cast<VariableNode*>(leftNode)) {
            std::string varName = varNode->varToken.literal;
            Value varValue = this->right->runNode(varScope);
            varScope.addVar(Variable {varName, varValue});
            return varValue;
        }
    }
    throw RunnerException("BO_Runner::runNode error: bin operator error at: {pos}", operToken);
}

Value RootNode::runNode(VariableScope& varScope) {
    for (const auto& instr : this->instructions) {
        Value result = instr->runNode(varScope);
        if (instr == instructions.back())
            cout << std::setprecision(17) << result << endl;
    }
    return NumberValue {0};
}

VariableScope Runner::run() {
    rootNode.runNode(varScope);
    return varScope;
}