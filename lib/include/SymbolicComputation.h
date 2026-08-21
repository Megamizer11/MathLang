// Выражения a = #LN 2 + #LN 6 Не вычисляется на прямую, а хранится ввиде: SUM(LN(2), LN(6))
// Тогда выражения b = e ^ a Не посчитает приближённый ответ, а выведет ровно 12
#pragma once

#include <memory>

#include "exceptions.h"
#include "RunValues.h"

namespace symcomp
{
    using longlong = long long;

    struct Base {
        virtual ~Base() = default;
        virtual longlong forcedCalc() const {
            throw RunnerException("Symbolic computation error: raw Base");
        };
    };

    struct Integer : Base {
        longlong arg;

        Integer(longlong val) : arg(val) {}

        longlong forcedCalc() const override {
            return this->arg;
        }
    };

    struct Add : Base {
        Base arg1;
        Base arg2;

        Add(Base val1, Base val2) : arg1(val1), arg2(val2) {}

        template<typename T1, typename T2>
        static Add get(const T1& left, const T2& right) {
            return Add {left, right};
        }

        static Integer get(const Integer& left, const Integer& right) {
            return Integer {left.arg + right.arg};
        }

        longlong forcedCalc() const override {
            return arg1.forcedCalc() + arg2.forcedCalc();
        }
    };

    struct Mult : Base {
        Base arg1;
        Base arg2;
    };

    struct Log : Base {};

    struct Root : Base {};
    
    struct Exponent : Base {};

    NumberValue toNumberValue(Base expr) {
        longlong solvedExpr = expr.forcedCalc();
        return NumberValue {solvedExpr, 0}.getNormalized();
    }

    // class SymbolicComputation {
    // public:
    //     SymbolicComputation() {

    //     }
    // };
}