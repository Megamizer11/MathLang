#pragma once

#include <string>
// #include <vector>
#include <iostream>
// #include <memory>
#include <type_traits>
#include <cmath>
#include <iomanip>

#include "AST.h"
#include "mpark/variant.hpp"
#include "exceptions.h"
#include "utils.h"

#define RETURN_NUMBERVALUE(rawMantissa, rawExponent) \
    /* В выражении 36 + 84 может получиться мантисса, равная 120 (это) */ \
    { if (rawMantissa == 0) return NumberValue {0, 0}; \
    long long _resultMantissa = rawMantissa; \
    long _resultExp = rawExponent; \
    while (_resultMantissa % 10 == 0) { \
        _resultMantissa = _resultMantissa / 10; \
        _resultExp++; \
    } \
    return NumberValue { \
        _resultMantissa, \
        _resultExp \
    };}

struct NumberValue {
    // Обычное число получается так: mantissa * 10^exponent
    long long mantissa;  // Для числа 12.345 это 12345, для 12000 это 12, мантисса может быть отрицательной, мантисса не может оканчиваться на ноль (исключение - мантисса равна нулю)
    long exponent;       // Для числа 12.345 это -3, для 12000 это 3, экспонента может быть отрицательной

    bool isInt() {
        return exponent >= 0;
    }
    
    std::string getAsString() const {
        return getLiteralFromMantissaAndExponent(mantissa, exponent);
    }

    long double asPrimitive() {
        return this->mantissa * pow(10, this->exponent);
    }

    NumberValue operator+(const NumberValue& rightNumberValue) {
        // Выражение 18*10^5 + 255*10^2 можно записать в виде: (18*10^(5-2) + 255*10^(2-2))*10^2 или: (18*10^3 + 255)*10^2 где 18*10^3 и 255 это и есть term1 и term2
        long minExp = std::min(this->exponent, rightNumberValue.exponent);
        long long term1 = std::round(this->mantissa * pow(10, (this->exponent - minExp)));  // без round иногда может возвращаться не 1100 (ожидаемое), а 1099
        long long term2 = std::round(rightNumberValue.mantissa * pow(10, (rightNumberValue.exponent - minExp)));
        long long rawMantissa = term1 + term2;  // В выражении 36 + 84 может получиться мантисса, равная 120 (хотя мантисса должна быть 12, а экспонента 1)
        long exp = minExp;
        RETURN_NUMBERVALUE(rawMantissa, exp)
    }

    NumberValue operator-(const NumberValue& rightNumberValue) {
        long minExp = std::min(this->exponent, rightNumberValue.exponent);
        long long term1 = std::round(this->mantissa * pow(10, (this->exponent - minExp)));
        long long term2 = std::round(rightNumberValue.mantissa * pow(10, (rightNumberValue.exponent - minExp)));
        long long rawMantissa = term1 - term2;
        long exp = minExp;
        RETURN_NUMBERVALUE(rawMantissa, exp)
    }

    NumberValue operator*(const NumberValue& rightNumberValue) {
        // С умножением всё проще, чем с делением: выражение 18*10^5 * 255*10^2 можно записать в виде: (18*255)*10^(5+2)
        long long rawMantissa = this->mantissa * rightNumberValue.mantissa;
        long exp = this->exponent + rightNumberValue.exponent;
        RETURN_NUMBERVALUE(rawMantissa, exp)
    }

    NumberValue operator/(const NumberValue& rightNumberValue) {
        // Разделить одно число на другое невозможно с бесконечной точностью. Поэтому мы превращаем результат деления в целое число, "раздувая" мантиссу с помощью pow(10, fixedDecimalAccuracy)
        // Выражение 18*10^5 / 255*10^2 можно записать в виде: (18/255)*10^10 * 10^(-10 + 5-2), где (18/255)*10^10 - целое (округлённое) число
        int fixedDecimalAccuracy = 10;
        long long inflatedMantissa = this->mantissa * pow(10, fixedDecimalAccuracy) / rightNumberValue.mantissa;
        long long rawMantissa = inflatedMantissa;
        long exp = -fixedDecimalAccuracy + this->exponent - rightNumberValue.exponent;
        RETURN_NUMBERVALUE(rawMantissa, exp)
    }

    NumberValue raiseToAPowerOf(const NumberValue& rightNumberValue) {
        // (5*10^3)^(4*10^6) = 5^(4*10^6) * 10^(3*4*10^6)
        int fixedDecimalAccuracy = 5;
        int accuracyCoeff = std::round(pow(10, fixedDecimalAccuracy));
        double expandedRightMantissa = (rightNumberValue.mantissa * pow(10, rightNumberValue.exponent));
        long long newMantissa = (accuracyCoeff * pow(this->mantissa, expandedRightMantissa));
        long newExponent = -fixedDecimalAccuracy + this->exponent * expandedRightMantissa;
        RETURN_NUMBERVALUE(newMantissa, newExponent)
        // return NumberValue {newMantissa, newExponent};
    }

    NumberValue getRemainder(const NumberValue& rightNumberValue) {  // Получить остаток от деления: 11 % 3 = 2
        // NumberValue getDiv = *this / rightNumberValue;
        // long long newMantissa = this->mantissa - (rightNumberValue.mantissa * getDiv.mantissa);
        // long newExponent = getDiv.exponent;
        // RETURN_NUMBERVALUE(newMantissa, newExponent)
        long long getDiv = this->mantissa / rightNumberValue.mantissa;
        long long newMantissa = this->mantissa - (rightNumberValue.mantissa * getDiv);
        long newExponent = this->exponent;
        RETURN_NUMBERVALUE(newMantissa, newExponent)
    }

    bool operator==(const NumberValue& rightNumberValue) const {
        return (this->mantissa == rightNumberValue.mantissa) && (this->exponent == rightNumberValue.exponent);
    };
};

class ExpressionNode;
class VariableNode;

struct FunctionValue {
    std::string name;
    ExpressionNode* body;
    std::vector<VariableNode*> args;

    // Value getValue(VariableScope& varScope) {
    //     return body->runNode(varScope);
    // }

    bool operator==(const FunctionValue& rightNumberValue) const {
        return false;  // Временно
    };
};

using Value = mpark::variant<
    NumberValue,
    FunctionValue
>;

inline std::ostream& operator<<(std::ostream& os, const NumberValue& val) {
    return os << val.getAsString();
}

inline std::ostream& operator<<(std::ostream& os, const Value& valueType) {
    mpark::visit([&os](const auto& val) {
        os << val;
    }, valueType);

    return os;
}

struct Variable {
    std::string name;
    Value value;
};

class VariableScope {
public:
    std::vector<Variable> vars;
    std::vector<FunctionValue> functions;

    VariableScope() {}

    void addVar(Variable var) {
        // vars.push_back(var);
        vars.insert(vars.begin(), var);  // Помогает создать иллюзию вложенности
    }

    const Variable* getByName(std::string name, const Token errorTooltipToken) {
        for (const Variable& var : this->vars) {
            if (var.name == name)
                return &var;
        }
        throw RunnerException("VariableScope::getByName error: variable {got_literal} was not declarated at {pos}", errorTooltipToken);
    }

    void addFunction(FunctionValue func) {
        functions.insert(functions.begin(), func);
    }

    const FunctionValue* getFunctionByName(std::string name, const Token errorTooltipToken) {
        for (const FunctionValue& func : this->functions) {
            if (func.name == name)
                return &func;
        }
        throw RunnerException("VariableScope::getFunctionByName error: function {got_literal} was not declarated at {pos}", errorTooltipToken);
    }
};