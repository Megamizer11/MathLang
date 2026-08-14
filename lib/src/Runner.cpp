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


// bool isNumber(const Value& val) {
//     return mpark::holds_alternative<IntValue>(val) || mpark::holds_alternative<DecimalValue>(val);
// }

// bool isDecimal(const Value& val) {
//     return mpark::holds_alternative<DecimalValue>(val);
// }

// bool isInteger(const Value& val) {
//     return mpark::holds_alternative<IntValue>(val);
// }

// DecimalValue asDecimalValue(const Value& val) {
//     if (const DecimalValue* valDec = mpark::get_if<DecimalValue>(&val)) {
//         return DecimalValue{valDec->integer, valDec->decimal};
//     } else if (const IntValue* valInt = mpark::get_if<IntValue>(&val)) {
//         return DecimalValue{valInt->value, 0};
//     } else {
//         throw runtime_error("BO_Runner > asFloatType error (must never happen): this value can\'t be float");
//     }
// }

// IntValue asIntegerValue(const Value& val) {
//     if (const IntValue* valInt = mpark::get_if<IntValue>(&val)) {
//         return IntValue{valInt->value};
//     } else {
//         throw runtime_error("BO_Runner > asFloatType error (must never happen): this value can\'t be float");
//     }
// }

// bool isNumber(const Value& val) {
//     return mpark::holds_alternative<NumberValue>(val);
// }

// float asFloatType(const Value& val) {
//     if (const NumberValue* valInt = mpark::get_if<NumberValue>(&val)) {
//         return (float)valInt->value;
//     } else {
//         throw runtime_error("BO_Runner > asFloatType error (must never happen): this value can\'t be float");
//     }
// }

Value BaseNode::runNode(VariableScope& varScope) {
    throw runtime_error("BaseNode::runNode error: BaseNode is not runnable");
}

Value ExpressionNode::runNode(VariableScope& varScope) {
    throw runtime_error("ExpressionNode::runNode error: ExpressionNode is not runnable");
}
 
Value NumberNode::runNode(VariableScope& varScope) {
    // return NumberValue { stof(this->numberToken.literal) };
    // if (this->isInt()) {
    //     return IntValue { this->getInt() };
    // }
    // return DecimalValue { this->getInt(), this->getDecimal() };
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
            // if (isNumber(left) && isNumber(right)) return NumberValue {asFloatType(left) + asFloatType(right)};
            // if (isDecimal(left) || isDecimal(right)) return asDecimalValue(left) + asDecimalValue(right);
            // if (isInteger(left) && isInteger(right)) return asIntegerValue(left) + asIntegerValue(right);
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