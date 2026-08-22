// Выражения a = #LN 2 + #LN 6 Не вычисляется на прямую, а хранится ввиде: SUM(LN(2), LN(6))
// Тогда выражения b = e ^ a Не посчитает приближённый ответ, а выведет ровно 12
#pragma once

#include <memory>

#include "exceptions.h"

// namespace symcomp
// {
//     using longlong = long long;

//     enum class Const {
//         E,
//         PI
//     };

//     struct Number {  // Число в самом чистом виде: рациональное, без периодической дроби
//         longlong mantissa;
//         long exponent;  // base10

//         // Number(longlong mantissa, long exponent) : mantissa(mantissa), exponent(exponent) {}
//     };

//     // struct NumberWrapper;

//     struct Base {
//         virtual ~Base() = default;
//         // Number forcedCalc() const override {}
//         virtual Number forcedCalc() const {
//             throw RunnerException("Symbolic computation error: raw Base");
//         };
//     };

//     struct NumberWrapper : Base {
//         Number num;

//         longlong& mantissa = num.mantissa;
//         long& exponent = num.exponent;

//         NumberWrapper(longlong mantissa, long exponent) {
//             num.mantissa = mantissa;
//             num.exponent = exponent;
//         }

//         NumberWrapper(Number num) {
//             this->num = num;
//         }

//         Number forcedCalc() const {
//             return Number {num.mantissa, num.exponent};
//             // return *this;
//             // return NumberWrapper(num.mantissa, num.exponent);
//         };
//     };

//     // Убирает нули у мантиссы: Number {1500, 3} -> Number {15, 5}
//     inline Number getNormalized(const Number& number) {
//         if (number.mantissa == 0) return Number {0, 0};
//         long long resultMantissa = number.mantissa;
//         long resultExp = number.exponent;
//         while (resultMantissa % 10 == 0) {
//             resultMantissa = resultMantissa / 10;
//             resultExp++;
//         }
//         return Number {resultMantissa, resultExp};
//     }

//     // Шаблон по умолчанию нужен для удобного использования статичных функций: symcomp::Add<>::get(add, i2);
//     // Данные для шаблона по умолчанию НЕ используются
//     template<typename T1 = Base, typename T2 = Base>
//     struct Add : Base {
//         // Base arg1;  // Если сделать такой тип, то произойдёт срезка объекта (object slicing), так как Base весит всего 4 байта
//         T1 arg1;
//         T2 arg2;

//         Add(Base val1, Base val2) : arg1(val1), arg2(val2) {}

//         template<typename TT1, typename TT2>
//         static Add get(const TT1& left, const TT2& right) {
//             return Add(left, right);
//         }

//         static Number get(const Number& left, const Number& right) {
//             // Выражение 18*10^5 + 255*10^2 можно записать в виде: (18*10^(5-2) + 255*10^(2-2))*10^2 или: (18*10^3 + 255)*10^2 где 18*10^3 и 255 это и есть term1 и term2
//             long minExp = std::min(left.exponent, right.exponent);
//             long long term1 = std::round(left.mantissa * pow(10, (left.exponent - minExp)));  // без round иногда может возвращаться не 1100 (ожидаемое), а 1099
//             long long term2 = std::round(right.mantissa * pow(10, (right.exponent - minExp)));
//             long long rawMantissa = term1 + term2;  // В выражении 36 + 84 может получиться мантисса, равная 120 (хотя мантисса должна быть 12, а экспонента 1)
//             long rawExp = minExp;
//             Number number = getNormalized(Number {rawMantissa, rawExp});
//             return number;
//         }

//         Number forcedCalc() const override {
//             return get(arg1.forcedCalc(), arg2.forcedCalc());
//         }
//     };

//     struct Mult : Base {
//         Base arg1;
//         Base arg2;
//     };

//     struct Log : Base {};

//     struct Root : Base {};
    
//     struct Exponent : Base {};
    
//     template<typename T>
//     struct Box : Base {
//         T value;

//         Number forcedCalc() const {
//             return value.forcedCalc();
//         }

//         // Box(T value) : value(std::move(value)) {}

//         Box(const T& value) : value(std::move(value)) {}
//     };

//     inline Base getSafeAnswer(const Base& expr) {
//         return expr;
//     }

//     inline NumberWrapper getSafeAnswer(const Number& num) {
//         return NumberWrapper {num};
//     }

//     // template<typename T1, typename T2>
//     // inline Add<T1, T2> toInnerData(const Add<T1, T2>& expr) {
//     //     return expr;
//     // }

//     // inline Add<Base, Base> toInnerData(const Add<Base, Base>* expr) {
//     //     return expr;
//     // }

//     inline Base& toInnerData(Base& expr) {
//         return expr;
//     }

//     inline Number toInnerData(const NumberWrapper& num) {
//         return Number {num.mantissa, num.exponent};
//     }
// }

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

    struct Add : Base {
        // Base arg1;  // Если сделать такой тип, то произойдёт срезка объекта (object slicing), так как Base весит всего 4 байта
        std::shared_ptr<Base> arg1;
        std::shared_ptr<Base> arg2;

        Add(Base val1, Base val2) {
            arg1 = std::make_shared<Base>(val1);
            arg2 = std::make_shared<Base>(val2);
        }

        Add(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2) {
            arg1 = val1;
            arg2 = val2;
        }

        template<typename TT1, typename TT2>
        static Add get(const TT1& left, const TT2& right) {
            return Add(left, right);
        }

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

    struct Log : Base {
        // std::shared_ptr<Base> arg;

        // Log(Number val) {
        //     arg = std::make_shared<Number>(val);
        // }

        // Log(Base val) {
        //     arg = std::make_shared<Base>(val);
        // }

        // Log(std::shared_ptr<Base> val) {
        //     arg = val;
        // }

        // template<typename TT1>
        // static Log get(const TT1& arg) {
        //     return Log(arg);
        // }

        // static Log get(const Number& arg) {
        //     return Log(arg);
        // }

        // static Number getAsNumber(const Number& arg) {  // Пока что не вычисляет логарифм
        //     Number number = getNormalized(Number {arg.mantissa+1, arg.exponent});
        //     return number;
        // }

        // Number forcedCalc() const override {
        //     return getAsNumber(arg->forcedCalc());
        // }
    };

    struct Root : Base {};
    
    struct Exponent : Base {};

    inline Base getSafeAnswer(const Base& expr) {
        return expr;
    }

    inline NumberWrapper getSafeAnswer(const Number& num) {
        return NumberWrapper {num};
    }

    // template<typename T1, typename T2>
    // inline Add<T1, T2> toInnerData(const Add<T1, T2>& expr) {
    //     return expr;
    // }

    // inline Add<Base, Base> toInnerData(const Add<Base, Base>* expr) {
    //     return expr;
    // }

    inline Base& toInnerData(Base& expr) {
        return expr;
    }

    inline Number toInnerData(const NumberWrapper& num) {
        return Number {num.mantissa, num.exponent};
    }
}