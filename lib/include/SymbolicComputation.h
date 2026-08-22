// Выражения a = #LN 2 + #LN 6 Не вычисляется на прямую, а хранится ввиде: SUM(LN(2), LN(6))
// Тогда выражения b = e ^ a Не посчитает приближённый ответ, а выведет ровно 12
#pragma once

#include <memory>
#include <type_traits>

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
        static long double getNaturalLoggedPrimitive(const Number& arg) {
            // ln(5*10^3) = ln5 + ln(10^3) = ln5 + 3ln10
            double log10e = log(10);
            long double lnMantissa = log10(arg.mantissa);
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

    struct Add : Base {
        // Base arg1;  // Если сделать такой тип, то произойдёт срезка объекта (object slicing), так как Base весит всего 4 байта
        std::shared_ptr<Base> arg1;
        std::shared_ptr<Base> arg2;

        // Add(Base val1, Base val2) {
        //     arg1 = std::make_shared<Base>(val1);
        //     arg2 = std::make_shared<Base>(val2);
        // }

        Add(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2) {
            arg1 = val1;
            arg2 = val2;
        }

        template<typename TT1, typename TT2>
        static Add get(const TT1& left, const TT2& right) {
            return Add(left, right);
        }
        
        std::shared_ptr<Base> formed() {
            if (NumberWrapper* leftNum = dynamic_cast<NumberWrapper*>(arg1.get())) {
                if (NumberWrapper* rightNum = dynamic_cast<NumberWrapper*>(arg2.get())) {
                    auto result = Add::get(leftNum->num, rightNum->num);
                    return std::make_shared<NumberWrapper>(result);
                }
            }

            if (Log* leftLog = dynamic_cast<Log*>(arg1.get())) {
                if (Log* rightLog = dynamic_cast<Log*>(arg2.get())) {
                    auto result = Number {leftLog->arg->forcedCalc().mantissa * rightLog->arg->forcedCalc().mantissa, 0};  // Временно, пока нет Mult
                    return std::make_shared<NumberWrapper>(result);
                }
            }

            return std::make_shared<Add>(this->arg1, this->arg2);
        }

        // Свойство логарифмов: log_a(b) + log_a(c) = log_a(b*c)
        // template<Log l1>
        // using suitable = std::enable_if_t<>
        // static Log getLogSum(const Log& left, const Log& right) {
        //     Mult logArg = Mult(left.arg, left.arg);
        //     Log out = Log();
        // }

        static Number get(const Number& left, const Number& right) {
            // Выражение 18*10^5 + 255*10^2 можно записать в виде: (18*10^(5-2) + 255*10^(2-2))*10^2 или: (18*10^3 + 255)*10^2 где 18*10^3 и 255 это и есть term1 и term2
            long minExp = std::min(left.exponent, right.exponent);
            long long term1 = std::round(left.mantissa * pow(10, (left.exponent - minExp)));  // без round иногда может возвращаться не 1100 (ожидаемое), а 1099
            long long term2 = std::round(right.mantissa * pow(10, (right.exponent - minExp)));
            long long rawMantissa = term1 + term2;  // В выражении 36 + 84 может получиться мантисса, равная 120 (хотя мантисса должна быть 12, а экспонента 1)
            long rawExp = minExp;
            Number number = getNormalized(Number {rawMantissa, rawExp});
            return number;
        }

        Number forcedCalc() const override {
            return get(arg1->forcedCalc(), arg2->forcedCalc());
        }
    };

    struct Mult : Base {
        Base arg1;
        Base arg2;
    };

    struct Root : Base {};
    
    struct Exponent : Base {};

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