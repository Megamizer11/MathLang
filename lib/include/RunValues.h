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

struct NumberValue {
    // Обычное число получается так: mantissa * 10^exponent
    long long mantissa;  // Для числа 12.345 это 12345, для 12000 это 12, мантисса может быть отрицательной
    long exponent;       // Для числа 12.345 это -3, для 12000 это 3, экспонента может быть отрицательной

    bool isInt() {
        return exponent >= 0;
    }
    
    // std::string getAsString() const {
    //     int mantissaLength = std::to_string(mantissa).length();  // учитывает минус
    //     int dotIndex = mantissaLength + exponent;
    //     std::string strMantissa = std::to_string(mantissa);
    //     std::cout << "mant " << mantissa << "  exp " << exponent << "  ln " << mantissaLength << std::endl;
    //     if (dotIndex >= 0 && exponent != 0)
    //         strMantissa.insert(dotIndex, ".");
    //     if (mantissa > 0 && dotIndex == 0)
    //         strMantissa.insert(dotIndex, "0");  // Без этого число 0.5 превратится в ".5"
    //     if (mantissa < 0 && dotIndex == 1)
    //         strMantissa.insert(dotIndex, "0");  // Без этого число -0.5 превратится в "-.5"
    //     return strMantissa;
    // }
    
    std::string getAsString() const {
        int mantissaLength = std::to_string(mantissa).length();  // учитывает минус
        int dotIndex = mantissaLength + exponent;
        std::string strMantissa = std::to_string(mantissa * pow(10, exponent));  // Очень сильно страдает точность: всего 6 значащих цифр
        return strMantissa;
    }

    NumberValue operator+(const NumberValue& rightNumberValue) {
        // Выражение 18*10^5 + 255*10^2 можно записать в виде: (18*10^(5-2) + 255*10^(2-2))*10^2 или: (18*10^3 + 255)*10^2 где 18*10^3 и 255 это и есть term1 и term2
        long minExp = std::min(this->exponent, rightNumberValue.exponent);
        long long term1 = std::round(this->mantissa * pow(10, (this->exponent - minExp)));  // без round иногда может возвращаться не 1100 (ожидаемое), а 1099
        long long term2 = std::round(rightNumberValue.mantissa * pow(10, (rightNumberValue.exponent - minExp)));
        return NumberValue {
            term1 + term2,
            minExp
        };
    }

    NumberValue operator-(const NumberValue& rightNumberValue) {
        long minExp = std::min(this->exponent, rightNumberValue.exponent);
        long long term1 = std::round(this->mantissa * pow(10, (this->exponent - minExp)));
        long long term2 = std::round(rightNumberValue.mantissa * pow(10, (rightNumberValue.exponent - minExp)));
        return NumberValue {
            term1 - term2,
            minExp
        };
    }

    NumberValue operator*(const NumberValue& rightNumberValue) {
        // С умножением всё проще, чем с делением: выражение 18*10^5 * 255*10^2 можно записать в виде: (18*255)*10^(5+2)
        return NumberValue {
            this->mantissa * rightNumberValue.mantissa,
            this->exponent + rightNumberValue.exponent
        };
    }

    NumberValue operator/(const NumberValue& rightNumberValue) {
        // Разделить одно число на другое невозможно с бесконечной точностью. Поэтому мы превращаем результат деления в целое число, "раздувая" мантиссу с помощью pow(10, fixedDecimalAccuracy)
        int fixedDecimalAccuracy = 10;
        long long inflatedMantissa = this->mantissa * pow(10, fixedDecimalAccuracy) / rightNumberValue.mantissa;
        return NumberValue {
            inflatedMantissa,
            -fixedDecimalAccuracy + this->exponent - rightNumberValue.exponent
        };
    }
};

using Value = mpark::variant<
    NumberValue
>;

inline std::ostream& operator<<(std::ostream& os, const NumberValue& val) {
    return os << val.getAsString();
    // return os << std::setprecision(15) << (val.mantissa * pow(10, val.exponent));
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

    VariableScope() {}

    void addVar(Variable var) {
        vars.push_back(var);
    }

    const Variable* getByName(std::string name, const Token errorTooltipToken) {
        for (const Variable& var : this->vars) {
            if (var.name == name)
                return &var;
        }
        throw RunnerException("VariableScope::getByName error: variable/function {got_literal} was not declarated at {pos}", errorTooltipToken);
    }
};