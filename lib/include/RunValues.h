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
    /* В выражении 36 + 84 может получиться мантисса, равная 120 (так быть не должно). Макрос преобразует мантиссу и экспоненту {120, 0} в  {12, 1} */ \
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

    long double asPrimitive() const {
        return this->mantissa * pow(10, this->exponent);
    }

    static NumberValue asNumberValue(long double num) {
        long exp = 10;  // Халтурный способ захардкодить экспоненту
        long long mantissa = std::round(num * pow(10, exp));
        RETURN_NUMBERVALUE(mantissa, -exp)
    }

    NumberValue getRounded(bool isDown) {  // isDown=true - округление вниз, isDown=false - округление вверх
        // Для числа 12.34 (NumberValue {1234, -2}) мы должны целочисленно разделить мантиссу на 100: 1234 / 100 = 12, это и будет мантисса нового числа. Новая экспонента будет равна нулю
        if (exponent >= 0)  // Гарантированно целое число
            return *this;  // Копируем текущий объект
        long long divFactor = std::round(pow(10, -this->exponent));
        long long newMantissa = this->mantissa / divFactor;  // Мантисса уже нормализована, она гарантированно не оканчивается на 0
        if (!isDown) newMantissa++;
        long newExponent = 0;
        return NumberValue {newMantissa, newExponent};  // Т.к. мантисса не оканчивается на 0, можно обойтись без макроса RETURN_NUMBERVALUE
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

    // NumberValue raiseToAPowerOf(const NumberValue& rightNumberValue) {
    //     // (5*10^3)^(4*10^6) = 5^(4*10^6) * 10^(3*4*10^6)
    //     // Но (3153*10^-3)^(45*10^-1) = 3153^4.5 * 10^(-3*)
    //     int fixedDecimalAccuracy = 0;
    //     int accuracyCoeff = std::round(pow(10, fixedDecimalAccuracy));
    //     double expandedRightMantissa = (rightNumberValue.mantissa * pow(10, rightNumberValue.exponent));
    //     long long newMantissa = std::round(accuracyCoeff * pow(this->mantissa, expandedRightMantissa));  // Это ужасная строчка.
    //     // Так как мы возводим мантиссу в степень, то при большой точности мантиссы с легкостью можно получить stack overflow и неправильный результат
    //     // 1.0001 ^ 4.8 при fixedDecimalAccuracy=0 уже вызывает переполнение стэка
    //     std::cout << "prim " << expandedRightMantissa << " " << newMantissa << std::endl;
    //     long newExponent = -fixedDecimalAccuracy + this->exponent * expandedRightMantissa;
    //     RETURN_NUMBERVALUE(newMantissa, newExponent)
    //     // return NumberValue {newMantissa, newExponent};
    // }

    NumberValue raiseToAPowerOf(const NumberValue& rightNumberValue) {
        // Из-за сильных проблем с точностью в предыдущей версии этой функции, она временно переделана
        long double thisExpanded = this->asPrimitive();
        long double rightExpanded = rightNumberValue.asPrimitive();
        long double result = pow(thisExpanded, rightExpanded);
        // std::cout << "AAAAAAAAA " << result << std::endl;
        return NumberValue::asNumberValue(result);
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

    long double _getNaturalLoggedPrimitive() {
        // ln(5*10^3) = ln5 + ln(10^3) = ln5 + 3ln10
        double log10e = log(10);
        long double lnMantissa = log(this->mantissa);
        long double lnExp = this->exponent * log10e;
        long double result = lnMantissa + lnExp;
        return result;
    }

    NumberValue getNaturalLogged() {
        long double primitiveNumber = _getNaturalLoggedPrimitive();
        int fixedDecimalAccuracy = 10;
        long long newMantissa = primitiveNumber * pow(10, fixedDecimalAccuracy);
        long newExp = -fixedDecimalAccuracy;
        RETURN_NUMBERVALUE(newMantissa, newExp)
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

// class VariableScope {
// public:
//     std::vector<Variable> vars;
//     std::vector<FunctionValue> functions;

//     VariableScope() {}

//     void printAllVars(int indent = 0) {
//         std::cout << std::string(indent, ' ') << "VARS: " << std::endl;
//         for (const Variable& var : this->vars) {
//             std::cout << std::string(indent + 2, ' ') << "name: " << var.name << ", value: " << var.value << std::endl;
//         }
//         std::cout << std::string(indent, ' ') << "PRINT VARS END" << std::endl;
//     }

//     void addVar(Variable var) {
//         // vars.push_back(var);
//         vars.insert(vars.begin(), var);  // Помогает создать иллюзию вложенности
//     }

//     void addVarRef(Variable& var) {
//         // vars.push_back(var);
//         vars.insert(vars.begin(), var);  // Помогает создать иллюзию вложенности
//     }

//     const Variable* getByName(std::string name, const Token errorTooltipToken) {
//         for (const Variable& var : this->vars) {
//             if (var.name == name)
//                 return &var;
//         }
//         throw RunnerException("VariableScope::getByName error: variable {got_literal} was not declarated at {pos}", errorTooltipToken);
//     }

//     void addFunction(FunctionValue func) {
//         functions.insert(functions.begin(), func);
//     }

//     const FunctionValue* getFunctionByName(std::string name, const Token errorTooltipToken) {
//         for (const FunctionValue& func : this->functions) {
//             if (func.name == name)
//                 return &func;
//         }
//         throw RunnerException("VariableScope::getFunctionByName error: function {got_literal} was not declarated at {pos}", errorTooltipToken);
//     }
// };

class VariableScope {
public:
    using shared_var = std::shared_ptr<Variable>;

    std::vector<shared_var> vars;
    std::vector<FunctionValue> functions;

    VariableScope() {}

    void printAllVars(int indent = 0) {
        std::cout << std::string(indent, ' ') << "VARS: " << std::endl;
        for (const shared_var& var : this->vars) {
            std::cout << std::string(indent + 2, ' ') << "name: " << var->name << ", value: " << var->value << std::endl;
        }
        std::cout << std::string(indent, ' ') << "PRINT VARS END" << std::endl;
    }

    void addVar(Variable var) {
        vars.insert(vars.begin(), std::make_shared<Variable>(var));
    }

    void addVar(std::shared_ptr<Variable> var) {
        // vars.push_back(var);
        vars.insert(vars.begin(), var);  // insert помогает создать иллюзию вложенности
    }

    const Variable* getByName(std::string name, const Token errorTooltipToken) {
        for (const shared_var& var : this->vars) {
            if (var->name == name)
                // return &*var;
                return var.get();
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