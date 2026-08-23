// Выражения a = #LN 2 + #LN 6 Не вычисляется на прямую, а хранится ввиде: SUM(LN(2), LN(6))
// Тогда выражения b = e ^ a Не посчитает приближённый ответ, а выведет ровно 12
#pragma once

#include <memory>
#include <type_traits>
#include <iostream>

#include "exceptions.h"

namespace symcomp
{
    using longlong = long long;

    enum class Const {
        E,
        PI
    };

    struct Number {  // Число в самом чистом виде: рациональное, без периодической дроби
        longlong mantissa;
        long exponent;  // base10

        // Number(longlong mantissa, long exponent) : mantissa(mantissa), exponent(exponent) {}
    };

    // struct NumberWrapper;

    struct Base {
        virtual ~Base() = default;
        // Number forcedCalc() const override {}
        virtual Number forcedCalc() const {
            throw RunnerException("Symbolic computation error: raw Base");
        };
    };

    struct NumberWrapper : Base {
        Number num;

        longlong& mantissa = num.mantissa;
        long& exponent = num.exponent;

        NumberWrapper(longlong mantissa, long exponent) {
            num.mantissa = mantissa;
            num.exponent = exponent;
        }

        NumberWrapper(Number num) {
            this->num = num;
        }

        Number forcedCalc() const {
            return Number {num.mantissa, num.exponent};
            // return *this;
            // return NumberWrapper(num.mantissa, num.exponent);
        };
    };

    // Убирает нули у мантиссы: Number {1500, 3} -> Number {15, 5}
    inline Number getNormalized(const Number& number) {
        if (number.mantissa == 0) return Number {0, 0};
        long long resultMantissa = number.mantissa;
        long resultExp = number.exponent;
        while (resultMantissa % 10 == 0) {
            resultMantissa = resultMantissa / 10;
            resultExp++;
        }
        return Number {resultMantissa, resultExp};
    }

    // Понижает точность мантиссы, что предотвращает переполнение стэка (pi: NumberValue{31415926535, -10} -> NumberValue{314, -2})
    inline Number getBalanced(const Number& num, long maxMantissaLength, bool rounding = false) {
        bool isNegative = num.mantissa < 0;
        long long newMantissa = num.mantissa;
        long newExponent = num.exponent;
        if (isNegative)
            newMantissa *= -1;
        long mantissaLen = floor(log10(newMantissa)) + 1;
        if (mantissaLen <= maxMantissaLength)
            return num;  // Создаёт копию. {23, 10}(4) -> {23, 10}
        long divFactor = mantissaLen - maxMantissaLength;
        int lastNumber;
        for (;divFactor >= 0; divFactor--) {
            lastNumber = newMantissa % 10;
            newMantissa = newMantissa / 10;  // Целочисленно делим мантиссу на 10 (31415 -> 3141)
            if (rounding && lastNumber >= 5)  // Окургление по правилам математики
                newMantissa++;
            newExponent++;  // Восстанавливаем порядок с помощью увеличения экспоненты

        }
        return Number {newMantissa, newExponent};
    }

    struct Add : Base {
        // Base arg1;  // Если сделать такой тип, то произойдёт срезка объекта (object slicing), так как Base весит всего 4 байта
        std::shared_ptr<Base> arg1;
        std::shared_ptr<Base> arg2;
        bool subtractionMode = false;  

        // Add(Base val1, Base val2) {
        //     arg1 = std::make_shared<Base>(val1);
        //     arg2 = std::make_shared<Base>(val2);
        // }

        Add(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2) {
            arg1 = val1;
            arg2 = val2;
        }
        
        std::shared_ptr<Base> formed();

        Number forcedCalc() const override {
            return get(arg1->forcedCalc(), arg2->forcedCalc(), subtractionMode);
        }
    
    private:
        // template<typename TT1, typename TT2>
        // static Add get(const TT1& left, const TT2& right, bool subtractionMode) {
        //     return Add(left, right, subtractionMode);
        // }

        static Number get(const Number& left, const Number& right, bool subtractionMode) {
            // Выражение 18*10^5 + 255*10^2 можно записать в виде: (18*10^(5-2) + 255*10^(2-2))*10^2 или: (18*10^3 + 255)*10^2 где 18*10^3 и 255 это и есть term1 и term2
            long minExp = std::min(left.exponent, right.exponent);
            long long term1 = std::round(left.mantissa * pow(10, (left.exponent - minExp)));  // без round иногда может возвращаться не 1100 (ожидаемое), а 1099
            long long term2 = std::round(right.mantissa * pow(10, (right.exponent - minExp)));
            if (subtractionMode)
                term2 *= -1;
            long long rawMantissa = term1 + term2;  // В выражении 36 + 84 может получиться мантисса, равная 120 (хотя мантисса должна быть 12, а экспонента 1)
            long rawExp = minExp;
            Number number = getNormalized(Number {rawMantissa, rawExp});
            return number;
        }
    };

    // struct Sub : Add {
    //     Subtr(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2) : Add(val1, val2) {}
    // };

    struct Mult : Base {
        std::shared_ptr<Base> arg1;
        std::shared_ptr<Base> arg2;

        Mult(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2) {
            arg1 = val1;
            arg2 = val2;
        }
        
        std::shared_ptr<Base> formed();

        Number forcedCalc() const override {
            return get(arg1->forcedCalc(), arg2->forcedCalc());
        }
    
    private:
        template<typename TT1, typename TT2>
        static Mult get(const TT1& left, const TT2& right) {
            return Mult(left, right);
        }

        static Number get(const Number& left, const Number& right) {
            // С умножением всё проще, чем с делением: выражение 18*10^5 * 255*10^2 можно записать в виде: (18*255)*10^(5+2)
            // Точность long long ~= 9.2*10^18, при умножении чисел, максимальная длина ответа (примерно) складывается из длин умножаемых чисел
            // Тогда, чтобы не было переполнения числа, каждой мантиссе стоит установить максимальную длину, равную 9. При сложении длина финальной мантиссы будет <=18, что не вызовет переполнение числа
            Number balancedThis = getBalanced(left, 9);
            Number balancedRight = getBalanced(right, 9);
            long long rawMantissa = balancedThis.mantissa * balancedRight.mantissa;
            long rawExp = balancedThis.exponent + balancedRight.exponent;
            Number number = getNormalized(Number {rawMantissa, rawExp});
            return number;
        }
    };

    struct Root : Base {};
    
    struct Exponent : Base {};

    struct Log : Base {
        std::shared_ptr<Base> arg;

        Log(Number val) {  // Т.к. мы не можем точно вычислить любой логарифм (например log(3)), то мы его сохраним в изнчальном виде
            arg = std::make_shared<NumberWrapper>(val);
        }

        Log(Base val) {
            arg = std::make_shared<Base>(val);
        }

        Log(std::shared_ptr<Base> val) {
            arg = val;
        }

        template<typename TT1>
        static Log get(const TT1& arg) {
            return Log(arg);
        }

        static Log get(const Number& arg) {
            return Log(arg);
        }

        static Number getAsNumber(const Number& arg) {
            return getNaturalLogged(arg);
        }

        Number forcedCalc() const override {
            return getAsNumber(arg->forcedCalc());
        }

    private:
        static constexpr double E_CONST = 2.71828182845904523536;

        static double logBaseN(double base, double x) {
            return log(x) / log(base);
        }

        static long double getNaturalLoggedPrimitive(const Number& arg) {
            // ln(5*10^3) = ln5 + ln(10^3) = ln5 + 3ln10
            double log10e = logBaseN(E_CONST, 10);
            long double lnMantissa = logBaseN(E_CONST, arg.mantissa);
            long double lnExp = arg.exponent * log10e;
            long double result = lnMantissa + lnExp;
            return result;
        }
    
        static Number getNaturalLogged(const Number& arg) {
            long double primitiveNumber = getNaturalLoggedPrimitive(arg);
            int fixedDecimalAccuracy = 10;
            long long newMantissa = primitiveNumber * pow(10, fixedDecimalAccuracy);
            long newExp = -fixedDecimalAccuracy;
            Number number = getNormalized(Number {newMantissa, newExp});
            return number;
        }
    };

    
    inline std::shared_ptr<Base> Add::formed() {
        if (NumberWrapper* leftNum = dynamic_cast<NumberWrapper*>(arg1.get())) {
            if (NumberWrapper* rightNum = dynamic_cast<NumberWrapper*>(arg2.get())) {
                auto result = Add::get(leftNum->num, rightNum->num, this->subtractionMode);
                return std::make_shared<NumberWrapper>(result);
            }
        }

        // Свойство логарифмов: log_a(b) + log_a(c) = log_a(b*c)
        if (Log* leftLog = dynamic_cast<Log*>(arg1.get())) {
            if (Log* rightLog = dynamic_cast<Log*>(arg2.get())) {
                // auto result = Number {leftLog->arg->forcedCalc().mantissa * rightLog->arg->forcedCalc().mantissa, 0};  // Временно, пока нет Mult
                auto result = Mult(leftLog->arg, rightLog->arg).formed();
                // std::cout << "MULT " << result->forcedCalc().mantissa << " " << result->forcedCalc().exponent << std::endl;
                return std::make_shared<Log>(result);
            }
        }

        return std::make_shared<Add>(this->arg1, this->arg2);
    }

    inline std::shared_ptr<Base> Mult::formed() {
        if (NumberWrapper* leftNum = dynamic_cast<NumberWrapper*>(arg1.get())) {
            if (NumberWrapper* rightNum = dynamic_cast<NumberWrapper*>(arg2.get())) {
                auto result = Mult::get(leftNum->num, rightNum->num);
                // std::cout << "MULT " << arg1->forcedCalc().mantissa << " " << arg2->forcedCalc().mantissa << " = " << result.mantissa << std::endl;
                // std::cout << "MULT " << std::endl;
                return std::make_shared<NumberWrapper>(result);
            }
        }

        return std::make_shared<Mult>(this->arg1, this->arg2);
    }


    inline Base getSafeAnswer(const Base& expr) {
        return expr;
    }

    inline NumberWrapper getSafeAnswer(const Number& num) {
        return NumberWrapper {num};
    }

    inline Base& toInnerData(Base& expr) {
        return expr;
    }

    inline Number toInnerData(const NumberWrapper& num) {
        return Number {num.mantissa, num.exponent};
    }
}