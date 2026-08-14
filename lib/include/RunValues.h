#pragma once

#include <string>
// #include <vector>
#include <iostream>
// #include <memory>
#include <type_traits>
#include <cmath>

#include "AST.h"
#include "mpark/variant.hpp"
#include "exceptions.h"

// DecimalValue& sumDecimals(const DecimalValue& firstDecimalValue, const DecimalValue& secondDecimalValue) {
//     DecimalValue result = {firstDecimalValue.integer, firstDecimalValue.decimal};
//     result.integer += secondDecimalValue.integer;

//     long long dec1 = firstDecimalValue.decimal;
//     long long dec2 = secondDecimalValue.decimal;
//     int order = floor(log10(dec1));
//     int order2 = floor(log10(dec2));
//     int firstOrderNumberDecimal = dec1 / order;  // При int / int это целочисленное деление. firstOrderNumber при 123.456 это 4
//     int firstOrderNumberDecimal2 = dec2 / order2;
//     if (order > order2)                         // Выравниваем дроби: у DecimalValue{2.34} и DecimalValue{1.2} значение decimal равно 34 и 2
//         dec2 = dec2 * pow(10, order - order2);  // Тогда нужно умножить вторую decimal на 10: 2*10 = 20, чтобы потом сложить дроби: 34+20 = 54
//     else if (order2 > order)
//         dec1 = dec1 * pow(10, order2 - order);
//     if (firstOrderNumberDecimal + firstOrderNumberDecimal2 > 9) {
//         result.integer += 1;
//         result.decimal = (dec1 + dec2) % (int)pow(10, abs(order - order2));
//     } else {
//         result.decimal += dec1 + dec2;
//     }
//     return result;
// }

// struct IntValue {
//     long long value;  // По хорошему значение нужно хранить в куче

//     // DecimalValue& operator+(const DecimalValue& rightDecimalValue) {  // нужно если будет реализовано Value + Value
//     //     return sumDecimals(DecimalValue {this->value, 0}, rightDecimalValue);
//     // }

//     IntValue& operator+(const IntValue& rightIntegerValue) {
//         return IntValue {this->value + rightIntegerValue.value};
//     }
// };

// struct DecimalValue {
//     long long integer;  // Сложно произовдить арифметические операции
//     long long decimal;  // с данными в таком виде
//     // long long value = ;
    
//     // long double getValue() { }
    
//     std::string getAsString() const {
//         return std::to_string(integer) + "." + std::to_string(decimal);
//     }

//     DecimalValue& operator+(const DecimalValue& rightDecimalValue) {
//         return sumDecimals(*this, rightDecimalValue);
//     }

//     // DecimalValue& operator+(const IntValue& rightIntegerValue) {  // нужно если будет реализовано Value + Value
//     //     return sumDecimals(*this, DecimalValue {rightIntegerValue.value, 0});
//     // }

//     // DecimalValue& operator+(const DecimalValue& newDecimalValue) {
//     //     DecimalValue result = {integer, decimal};
//     //     result.integer += newDecimalValue.integer;

//     //     // const long long& dec2 = newDecimalValue.decimal;
//     //     // int order = floor(log10(this->decimal));
//     //     // int order2 = floor(log10(dec2));
//     //     // int firstOrderNumberDecimal = decimal / order;  // При int / int это целочисленное деление. firstOrderNumber при 123.456 это 4
//     //     // int firstOrderNumberDecimal2 = dec2 / order2;
//     //     // if (firstOrderNumberDecimal + firstOrderNumberDecimal2 > 9) {
//     //     //     result.integer += 1;
//     //     //     //
//     //     // } else {
//     //     //     result.decimal += this->decimal + dec2;
//     //     // }
//     //     long long dec1 = this->decimal;
//     //     long long dec2 = newDecimalValue.decimal;
//     //     int order = floor(log10(dec1));
//     //     int order2 = floor(log10(dec2));
//     //     int firstOrderNumberDecimal = decimal / order;  // При int / int это целочисленное деление. firstOrderNumber при 123.456 это 4
//     //     int firstOrderNumberDecimal2 = dec2 / order2;
//     //     if (order > order2)                         // Выравниваем дроби: у DecimalValue{2.34} и DecimalValue{1.2} значение decimal равно 34 и 2
//     //         dec2 = dec2 * pow(10, order - order2);  // Тогда нужно умножить вторую decimal на 10: 2*10 = 20, чтобы потом сложить дроби: 34+20 = 54
//     //     else if (order2 > order)
//     //         dec1 = dec1 * pow(10, order2 - order);
//     //     if (firstOrderNumberDecimal + firstOrderNumberDecimal2 > 9) {
//     //         result.integer += 1;
//     //         result.decimal = (dec1 + dec2) % (int)pow(10, abs(order - order2));
//     //     } else {
//     //         result.decimal += dec1 + dec2;
//     //     }
//     //     return result;
//     // }
// };

struct NumberValue {
    // Обычное число получается так: mantissa * 10^exponent
    long long mantissa;  // Для числа 12.345 это 12345, для 12000 это 12, мантисса может быть отрицательной
    long exponent;       // Для числа 12.345 это -3, для 12000 это 3, экспонента может быть отрицательной

    bool isInt() {
        return exponent >= 0;
    }
    
    std::string getAsString() const {
        // int mantissaLength = floor(log10(mantissa)) + 1;
        int mantissaLength = std::to_string(mantissa).length();  // учитывает минус
        // std::cout << "mant " << mantissa << "  exp " << exponent << "  ln " << mantissaLength << std::endl;
        int dotIndex = mantissaLength + exponent;
        std::string strMantissa = std::to_string(mantissa);
        if (dotIndex >= 0 && exponent != 0)
            strMantissa.insert(dotIndex, ".");
        if (mantissa > 0 && dotIndex == 0)
            strMantissa.insert(dotIndex, "0");  // Без этого число 0.5 превратится в ".5"
        if (mantissa < 0 && dotIndex == 1)
            strMantissa.insert(dotIndex, "0");  // Без этого число -0.5 превратится в "-.5"
        return strMantissa;
    }

    NumberValue operator+(const NumberValue& rightNumberValue) {
        // Выражение 18*10^5 + 255*10^2 можно записать в виде: (18*10^(5-2) + 255*10^(2-2))*10^2 или: (18*10^3 + 255)*10^2 где 18*10^3 и 255 это и есть term1 и term2
        long minExp = std::min(this->exponent, rightNumberValue.exponent);
        // long long term1 = this->mantissa * std::round(pow(10, (rightNumberValue.exponent + minExp)));  // обычное (int)pow здесь нельзя использовать, т.к. вместо при (int)pow(10, 2) может иногда возвращаться 99, а не 100
        long long term1 = std::round(this->mantissa * pow(10, (this->exponent - minExp)));  // обычное (int)pow здесь нельзя использовать, т.к. вместо при (int)pow(10, 2) может иногда возвращаться 99, а не 100
        long long term2 = std::round(rightNumberValue.mantissa * pow(10, (rightNumberValue.exponent - minExp)));
        // std::cout << this->exponent - minExp << " " << pow(10, (this->exponent - minExp)) << " t1 " << term1 << "  t2 " << term2 << std::endl;
        // std::cout << std::round(pow(10, (this->exponent - minExp))) << std::endl;
        // std::cout << rightNumberValue.mantissa << "*" << pow(10, (rightNumberValue.exponent - minExp)) << " PLUS " << this->mantissa << " " << rightNumberValue.mantissa << " " << term1 << " " << term2 << " " << minExp << std::endl;
        return NumberValue {
            // this->mantissa * (int)pow(10, this->exponent) + rightNumberValue.mantissa * (int)pow(10, rightNumberValue.exponent)
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
            // (long long)(inflatedMantissa * pow(10, (this->mantissa - rightNumberValue.mantissa) - fixedDecimalAccuracy)),
            inflatedMantissa,
            fixedDecimalAccuracy
        };
    }
};

// struct NumberValue {
//     float value;
// };

using Value = mpark::variant<
    // IntValue,
    // DecimalValue
    NumberValue
>;

// struct Value {
//     RawValue rawValue;

//     // template<typename T>
//     // inline constexpr lib::add_pointer_t<variant_alternative_t<I, variant<Ts...>>> get_if() {
//     template <std::size_t I, typename... Ts>
//     inline constexpr std::add_pointer_t<mpark::variant_alternative<I, mpark::variant<Ts...>>>::type get_if() {
//         return mpark::get_if<T>(&rawValue);
//     }

//     // template<typename T>
//     // inline constexpr const T get_if() {
//     //     return mpark::get_if<T>(&rawValue);
//     // }

//     template <std::size_t I, typename... Ts>
//     inline constexpr std::add_pointer<const mpark::variant_alternative_t<I, mpark::variant<Ts...>>>::type get_if() {
//         return mpark::get_if<T>(&rawValue);
//     }
// };

// inline std::string as_string(const Value& val) {  /// Должна конвертировать абсолютно любой Value в строку
//     if (const NumberValue* valInt = mpark::get_if<NumberValue>(&val)) {
//         return std::to_string(valInt->value);
//     }
//     throw RunnerException("RunValues > as_string error (that must never happen): this value can\'t be string");
// }

// inline std::ostream& operator<<(std::ostream& os, const IntValue& val) {
//     return os << val.value;
// }

// inline std::ostream& operator<<(std::ostream& os, const DecimalValue& val) {
//     return os << val.getAsString();
// }

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