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
#include "SymbolicComputation.h"

#define PRINT_PRECISION 8

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


// template<typename T>
struct NumberValue {
    // Обычное число получается так: mantissa * 10^exponent
    long long mantissa;  // Для числа 12.345 это 12345, для 12000 это 12, мантисса может быть отрицательной, мантисса не может оканчиваться на ноль (исключение - мантисса равна нулю)
    long exponent;       // Для числа 12.345 это -3, для 12000 это 3, экспонента может быть отрицательной

    std::shared_ptr<symcomp::Base> inner = nullptr;

    NumberValue(const symcomp::Number& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::NumberWrapper>(tree);
    };

    NumberValue(const symcomp::NumberWrapper& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::NumberWrapper>(tree);
    };

    NumberValue(const symcomp::Add& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::Add>(tree);
    };

    NumberValue(const symcomp::Exponent& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::Exponent>(tree);
    };

    NumberValue(const symcomp::Log& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::Log>(tree);
    };

    NumberValue(const std::shared_ptr<symcomp::Base> tree) {  // По факту внутренний API для символьного вычисления
        inner = tree;
    };

    NumberValue(long long mantissa, long exponent) : mantissa(mantissa), exponent(exponent) {  // Базовый конструктор
        inner = std::make_shared<symcomp::NumberWrapper>(mantissa, exponent);
    };

    bool isInt() {
        return exponent >= 0;
    }
    
    // std::string getAsString() const {
    //     return getLiteralFromMantissaAndExponent(mantissa, exponent);
    // }
    
    static std::string getAsString(symcomp::Number num) {
        return getLiteralFromMantissaAndExponent(num.mantissa, num.exponent);
    }

    long double asPrimitive() const {
        symcomp::Number answer = this->inner->forcedCalc();
        return answer.mantissa * pow(10, answer.exponent);
    }

    // static NumberValue asNumberValue(long double num) {
    //     long exp = 10;  // Халтурный способ захардкодить экспоненту
    //     long long mantissa = std::round(num * pow(10, exp));
    //     RETURN_NUMBERVALUE(mantissa, -exp)
    // }

    // symcomp::Number asNumber() const {
    //     return symcomp::Number {this->mantissa, this->exponent};
    // }

    // // Понижает точность мантиссы, что предотвращает переполнение стэка (pi: NumberValue{31415926535, -10} -> NumberValue{314, -2})
    // NumberValue getBalanced(long maxMantissaLength, bool rounding = false) const {
    //     bool isNegative = this->mantissa < 0;
    //     long long newMantissa = this->mantissa;
    //     long newExponent = this->exponent;
    //     if (isNegative)
    //         newMantissa *= -1;
    //     long mantissaLen = floor(log10(newMantissa)) + 1;
    //     if (mantissaLen <= maxMantissaLength)
    //         return *this;  // {23, 10}(4) -> {23, 10}
    //     long divFactor = mantissaLen - maxMantissaLength;
    //     int lastNumber;
    //     for (;divFactor >= 0; divFactor--) {
    //         lastNumber = newMantissa % 10;
    //         newMantissa = newMantissa / 10;  // Целочисленно делим мантиссу на 10 (31415 -> 3141)
    //         if (rounding && lastNumber >= 5)  // Окургление по правилам математики
    //             newMantissa++;
    //         newExponent++;  // Восстанавливаем порядок с помощью увеличения экспоненты

    //     }
    //     return NumberValue {newMantissa, newExponent};
    // }

    // NumberValue getNormalized() {
    //     if (this->mantissa == 0) return NumberValue {0, 0};
    //     long long resultMantissa = this->mantissa;
    //     long resultExp = this->exponent;
    //     while (resultMantissa % 10 == 0) {
    //         resultMantissa = resultMantissa / 10;
    //         resultExp++;
    //     }
    //     return NumberValue {resultMantissa, resultExp};
    // }

    NumberValue getRounded(bool isDown) {  // isDown=true - округление вниз, isDown=false - округление вверх
        return NumberValue(symcomp::getRounded(this->inner->forcedCalc(), isDown));
    }

    NumberValue operator+(const NumberValue& rightNumberValue) {
        auto result = symcomp::Add(
            this->inner,
            rightNumberValue.inner,
            false
        ).formed();
        return NumberValue(result);
    }

    NumberValue operator-(const NumberValue& rightNumberValue) {
        auto result = symcomp::Add(
            this->inner,
            rightNumberValue.inner,
            true
        ).formed();
        return NumberValue(result);
    }

    NumberValue operator*(const NumberValue& rightNumberValue) {
        auto result = symcomp::Mult(
            this->inner,
            rightNumberValue.inner,
            false
        ).formed();
        return NumberValue(result);
    }

    NumberValue operator/(const NumberValue& rightNumberValue) {
        auto result = symcomp::Mult(
            this->inner,
            rightNumberValue.inner,
            true
        ).formed();
        return NumberValue(result);
    }

    NumberValue raiseToAPowerOf(const NumberValue& rightNumberValue) {
        auto result = symcomp::Exponent(
            this->inner,
            rightNumberValue.inner
        ).formed();
        return NumberValue(result);
    }

    NumberValue getRemainder(const NumberValue& rightNumberValue) {  // Получить остаток от деления: 11 % 3 = 2
        long long getDiv = this->mantissa / rightNumberValue.mantissa;
        long long newMantissa = this->mantissa - (rightNumberValue.mantissa * getDiv);
        long newExponent = this->exponent;
        RETURN_NUMBERVALUE(newMantissa, newExponent)
    }

    // long double _getNaturalLoggedPrimitive() {
    //     // ln(5*10^3) = ln5 + ln(10^3) = ln5 + 3ln10
    //     double log10e = log(10);
    //     long double lnMantissa = log(this->mantissa);
    //     long double lnExp = this->exponent * log10e;
    //     long double result = lnMantissa + lnExp;
    //     return result;
    // }

    // NumberValue getNaturalLogged() {
    //     long double primitiveNumber = _getNaturalLoggedPrimitive();
    //     int fixedDecimalAccuracy = 10;
    //     long long newMantissa = primitiveNumber * pow(10, fixedDecimalAccuracy);
    //     long newExp = -fixedDecimalAccuracy;
    //     RETURN_NUMBERVALUE(newMantissa, newExp)
    // }

    NumberValue getNaturalLogged() {
        auto result = symcomp::Log(
            this->inner
        ).formed();
        return NumberValue(result);
    }

    bool operator==(const NumberValue& rightNumberValue) const {
        symcomp::Number forcedCalcThis = this->inner->forcedCalc();
        symcomp::Number forcedCalcRight = this->inner->forcedCalc();
        if (forcedCalcThis.mantissa == 0 && forcedCalcRight.mantissa == 0)
            return true;
        return (forcedCalcThis.mantissa == forcedCalcRight.mantissa) && (forcedCalcThis.exponent == forcedCalcRight.exponent);
    };

    void printAsSymbolicTree() {
        this->inner->printTree();
    }
};

class ExpressionNode;
class VariableNode;

struct FunctionValue {
    std::string name;
    ExpressionNode* body;
    std::vector<VariableNode*> args;

    bool operator==(const FunctionValue& rightNumberValue) const {
        return false;  // Временно
    };
};

using Value = mpark::variant<
    NumberValue,
    FunctionValue
>;

inline std::ostream& operator<<(std::ostream& os, const NumberValue& val) {
    // return os << val.getBalanced(PRINT_PRECISION, true)  // 1.99999982358225 -> 2.000
    //                 .getNormalized()                     // 2.000 -> 2
    //                 .getAsString();                      // 2 -> "2"
    // if (val.inner != 0) {
    //     // std::cout << val.inner << std::endl;
    //     auto answer = val.inner->forcedCalc();
    //     return os << "NUMVAL = " << NumberValue::getAsString(answer) << " | " << answer.mantissa << " " << answer.exponent << " | " << val.mantissa << " " << val.exponent;
    // }
    // return os << "NUMVAL2 " << val.mantissa << " " << val.exponent;
    if (val.inner != 0) {
        auto answer = val.inner->forcedCalc();
        std::string strAnswer = NumberValue::getAsString(answer);
        if (strAnswer.length() > 100000)
            throw RunnerException("NumberValue operator<< error: too large value for printing");
        return os << strAnswer;
    }
    throw RunnerException("NumberValue operator<< error: somehow inner (ptr) == 0");
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