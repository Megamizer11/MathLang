// Выражения a = #LN 2 + #LN 6 Не вычисляется на прямую, а хранится ввиде: SUM(LN(2), LN(6))
// Тогда выражения b = e ^ a Не посчитает приближённый ответ, а выведет ровно 12
#pragma once

#include <memory>
#include <type_traits>
#include <algorithm>
#include <iostream>

#include "exceptions.h"

// Все static Number get функции применяются в двух случаях: когда выражение можно упростить без потери точности: 2^3 = 8
// или когда выражение должно быть полностью просчитано, например при выводе на экран: 1/3 должно превратиться в 0.3333333

// Все formed функции нужны для попытки упрощения выражения: Add(1, 2).formed() => 3
// если выражение нельзя упростить, оно не меняется: Log(2).formed() => Log(2)

// get приеняется как конечное упрощение, formed внутри formed как незаконченное упрощение

// make_shared внутри formed можно использовать только в двух случаях: make_shared<NumberWrapper> или в самом конце функции, при неудачном упрощении

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
    };

    struct Base {
        virtual ~Base() = default;
        // Number forcedCalc() const override {}
        virtual Number forcedCalc() const {
            throw RunnerException("Symbolic computations forcedCalc() error: raw Base");
        };
        virtual void printTree(int indent = 0) {
            throw RunnerException("Symbolic computations printTree() error: raw Base");
        }
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
        };

        void printTree(int indent = 0) {
            std::cout << std::string(indent*2, ' ') << "NUMBER: " << getLiteralFromMantissaAndExponent(num.mantissa, num.exponent) << std::endl;
        }
    };
    
    struct ConstWrapper : Base {
        Const constType;

        ConstWrapper(Const constType) {
            this->constType = constType;
        }

        Number forcedCalc() const {
            switch (constType) {
                case Const::E:
                    return Number {2718281828459045, -15};
                case Const::PI:
                    return Number {3141592653589793, -15};
                default:
                    throw RunnerException("Symbolic computations ConstWrapper error: undefined const \"{0}\"", static_cast<int>(constType));
            }
        };

        void printTree(int indent = 0) {
            std::string name = "NONE";
            switch (constType) {
                case Const::E: name = "E";
                case Const::PI: name = "PI";
                default: name = "NONE";
            }
            std::cout << std::string(indent*2, ' ') << "CONST: " << name << std::endl;
        }
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

    // Понижает точность мантиссы, что предотвращает переполнение стэка (pi: Number{31415926535, -10} -> Number{314, -2})
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

    // Равносильно 1/num (Number{5, 1} => Number{2, -2})
    inline Number getInversed(const Number& num) {
        // В отличие от выражения num1/num2, нам не нужно заботиться о точности при делении мантиссы num1 на num2, достаточно выставить максимальное округлённое значение для long long (это 10^18)
        long long inversedMantissa = pow(10, 18) / num.mantissa;
        long inversedExponent = -18 - num.exponent;
        Number result = getNormalized(Number {inversedMantissa, inversedExponent});
        return result;
    }

    inline Number getRounded(const Number& num, bool isDown) {  // isDown=true - округление вниз, isDown=false - округление вверх
        // Для числа 12.34 (NumberValue {1234, -2}) мы должны целочисленно разделить мантиссу на 100: 1234 / 100 = 12, это и будет мантисса нового числа. Новая экспонента будет равна нулю
        if (num.exponent >= 0)  // Гарантированно целое число
            return num;  // Копируем текущий объект
        long long divFactor = std::round(pow(10, num.exponent));
        long long newMantissa = num.mantissa / divFactor;  // Мантисса уже нормализована, она гарантированно не оканчивается на 0
        if (!isDown) newMantissa++;
        long newExponent = 0;
        return Number {newMantissa, newExponent};  // Т.к. мантисса не оканчивается на 0, можно обойтись без функции getNormalized
    }

    inline bool equals(const Number& num1, const Number& num2) {
        if (num1.mantissa == 0 && num2.mantissa == 0)
            return true;
        return (num1.mantissa == num2.mantissa) && (num1.exponent == num2.exponent);
    };

    inline bool equals(std::shared_ptr<Base> expr1, std::shared_ptr<Base> expr2) {
        symcomp::Number forcedCalcThis = expr1->forcedCalc();
        symcomp::Number forcedCalcRight = expr2->forcedCalc();
        if (forcedCalcThis.mantissa == 0 && forcedCalcRight.mantissa == 0)
            return true;
        return (forcedCalcThis.mantissa == forcedCalcRight.mantissa) && (forcedCalcThis.exponent == forcedCalcRight.exponent);
    };

    inline Number getRemainder(const Number& num1, const Number& num2) {  // Получить остаток от деления: 11 % 3 = 2
        long long getDiv = num1.mantissa / num2.mantissa;
        long long newMantissa = num1.mantissa - (num2.mantissa * getDiv);
        long newExponent = num1.exponent;
        Number result = getNormalized(Number {newMantissa, newExponent});
        return result;
    }

    inline bool isInt(long double num) {
        return std::trunc(num) == num;
    }

    // Это должен быть признак Паскаля. Но проще разделить n на m и проверить, является ли результат целым числом
    // inline long long fullDivisibilityTest(long long n, long long m) {  // Признак делимости n на m, m - любое целое число
    //     int lenOfN = floor(log10(n)) + 1;
    // }

    struct Add : Base {
        // Base arg1;  // Если сделать такой тип, то произойдёт срезка объекта (object slicing), так как Base весит всего 4 байта
        std::shared_ptr<Base> arg1;
        std::shared_ptr<Base> arg2;
        bool subtractionMode;

        Add(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2, bool subtractionMode) {
            arg1 = val1;
            arg2 = val2;
            this->subtractionMode = subtractionMode;
        }
        
        std::shared_ptr<Base> formed();

        Number forcedCalc() const override {
            return get(arg1->forcedCalc(), arg2->forcedCalc(), subtractionMode);
        }

        void printTree(int indent = 0) {
            std::string operName = (subtractionMode ? "SUBTRACT" : "ADD");
            std::cout << std::string(indent*2, ' ') << operName << std::endl;
            arg1->printTree(indent+2);
            arg2->printTree(indent+2);
        }
    
    private:
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

    struct Mult : Base {
        std::shared_ptr<Base> arg1;
        std::shared_ptr<Base> arg2;
        bool divisionMode;

        Mult(std::shared_ptr<Base> val1, std::shared_ptr<Base> val2, bool divisionMode) {
            arg1 = val1;
            arg2 = val2;
            this->divisionMode = divisionMode;
        }
        
        std::shared_ptr<Base> formed();

        Number forcedCalc() const override {
            return get(arg1->forcedCalc(), arg2->forcedCalc(), divisionMode);
        }

        void printTree(int indent = 0) {
            std::string operName = (divisionMode ? "DIVIDE" : "MULT");
            std::cout << std::string(indent*2, ' ') << operName << std::endl;
            arg1->printTree(indent+2);
            arg2->printTree(indent+2);
        }
    
    private:
        static Number get(const Number& left, const Number& right, bool divisionMode) {
            // С умножением всё проще, чем с делением: выражение 18*10^5 * 255*10^2 можно записать в виде: (18*255)*10^(5+2)
            // Точность long long ~= 9.2*10^18, при умножении чисел, максимальная длина ответа (примерно) складывается из длин умножаемых чисел
            // Тогда, чтобы не было переполнения числа, каждой мантиссе стоит установить максимальную длину, равную 9. При сложении длина финальной мантиссы будет <=18, что не вызовет переполнение числа
            Number balancedLeft = getBalanced(left, 9);
            Number balancedRight = getBalanced(right, 9);
            long long rawMantissa;
            if (divisionMode) {
                long double floatDivision = static_cast<double>(balancedLeft.mantissa) / balancedRight.mantissa;
                if (std::trunc(floatDivision) == floatDivision) {  // Если левое число делится на правое без остатка (48/6)
                    // В этом случае getInversed применять нельзя, т.к. getInversed(Number{6, 0}) это getInversed(Number{0.1(6), 0}), т.е. бесконечная периодическая дробь, которая не может обеспечить идеальную точность
                    rawMantissa = balancedLeft.mantissa / balancedRight.mantissa;
                } else {  // Числа не делятся нацело (2/7), тогда нужно поделить их с максимально возможной (но не идеальной) точностью
                    balancedRight = getInversed(balancedRight);
                    rawMantissa = balancedLeft.mantissa * balancedRight.mantissa;
                }
            } else
                rawMantissa = balancedLeft.mantissa * balancedRight.mantissa;
            long rawExp = balancedLeft.exponent + balancedRight.exponent;
            Number number = getNormalized(Number {rawMantissa, rawExp});
            return number;
        }
    };

    // struct Root : Base {};
    
    struct Exponent : Base {
        std::shared_ptr<Base> base;
        std::shared_ptr<Base> exp;

        Exponent(std::shared_ptr<Base> base, std::shared_ptr<Base> exp) {
            this->base = base;
            this->exp = exp;
        }
        
        std::shared_ptr<Base> formed();

        bool isPurelyComputable() {  // возвращает true, только если выражение может быть моментально посчитано без потери точности
            return true;
        }

        Number forcedCalc() const override {
            return get(base->forcedCalc(), exp->forcedCalc());
        }

        void printTree(int indent = 0) {
            std::cout << std::string(indent*2, ' ') << "EXP" << std::endl;
            base->printTree(indent+2);
            exp->printTree(indent+2);
        }
    
    private:
        // Превращает дробное число в целое с сохранением точности (makeNumber(12.34, 0) -> Number{1234, -2})
        static Number makeNumber(long double mantissa, long exp) {
            std::string numAsStr = std::to_string(mantissa);  // 12.34 -> "12.340000"
            numAsStr.erase(numAsStr.find_last_not_of('0') + 1, std::string::npos);  // "12.340000" -> "12.34"
            if(numAsStr.back() == '.') numAsStr.pop_back();  // Удаляем точку на конце, если она есть. Она появится, если дать на вход целое число
            if (numAsStr.find('.') != std::string::npos) {  // Для 12.345 exp уменьшается на 3
                exp -= (numAsStr.length() - 1) - numAsStr.find('.');
            }
            numAsStr.erase(std::remove(numAsStr.begin(), numAsStr.end(), '.'), numAsStr.end());  // "12.34" -> "1234"
            // std::cout << "STOLL " << numAsStr << " M: " << mantissa << " E: " << exp << std::endl;
            return Number {std::stoll(numAsStr), exp};  // "1234" -> 1234
        }

        // Этот алгоритм неидеален, так как он очень тяжелый и должен применяться только в крайних случаях, а для обычного 2^3 применять логарифмы не лучшая идея
        // Использует алгоритм вычисления степени через логарифмы
        // На примере выражения 3^183: Сначала выражение нужно логарифмировать (например по основанию 10): lg(3^183) = 183*lg3 = 87.31389
        // Т.к. 3^183 = 10^lg(3^183), то 3^183 = 10^87.31389. Далее нужно разделить на целую и дробную часть: 10^87.31389 = 10^(0.31389+87) = 10^0.31389 * 10^87
        // Считаем первую часть: 10^0.31389 = 2.0567, тогда 3^183 = 10^0.31389 * 10^87 = 2.0567 * 10^87. Это и есть ответ
        static Number calculationWithLogarithms(const Number& base, const Number& exp) {
            // На примере выражения: (3*10^7) ^ (25*10^-1) = 4.929503 * 10^18
            if (base.mantissa == 0)
                return Number {0, 0};
            bool isNegativeBase = base.mantissa < 0;
            long double expandedExp = exp.mantissa * pow(10, exp.exponent);  // Разворачиваем правую часть: expandedExp = 2.5

            long long baseMantissa = base.mantissa;
            if (isNegativeBase) {  // Если основание отрицательное, то алгоритм становится сложнее, так как log10 при вычислении baseLoged использовать просто так нельзя
                if (std::trunc(expandedExp) == expandedExp) {  // Степень целая, выражение спокойно считается
                    baseMantissa *= -1;  // Делаем мантиссу положительной, чтобы log10 не вызывал ошибок, в конце сделаем обратное действие
                } else {
                    // Учтены не все случаи: (-8)^(1/3) кидает ошибку, хотя это выражение равно 2.
                    throw RunnerException("Symbolic computations Math error: wrong exponent expression");
                }
            }

            long double baseLoged = (std::log10(baseMantissa) + base.exponent);  // Логарифмируем основание: lg(3*10^7) = lg3 + lg(10^7) = lg3 + 7 = 7.477121
            long double logedResultMantissa = expandedExp * baseLoged;  // Получаем логарифмированное полное выражение: lg ((3*10^7) ^ (25*10^-1)) = (25*10^-1)*lg(3*10^7) = 2.5*7.477121 = 18.69280314
            long intLogedResultMantissa = std::trunc(logedResultMantissa);  // Целая часть от 18.69280314 = 18
            double fractionOfLogedResultMantissa = logedResultMantissa - intLogedResultMantissa;  // Дробь от 18.69280314 = 0.69280314  // От 0 до 1
            double floatResultMantissa = pow(10, fractionOfLogedResultMantissa);  // 10^0.69280314 = 4.929503  // От 0 до 10
            // Вообще это действие немного волшебное, так как fractionOfLogedResultMantissa почти гарантированно дробное, но из-за очень большой точности при возведении 10 в степень fractionOfLogedResultMantissa, результат округляется до нужных значений

            long rawFinalExp = intLogedResultMantissa;  // 18 
            // std::cout << baseMantissa << " " << std::log10(baseMantissa) << " " << floatResultMantissa << " " << rawFinalExp << std::endl;

            if (isNegativeBase) {
                if ((long long)std::trunc(expandedExp) % 2 == 1) {  // Если степень нечетная, обратно меняем положительный знак на отрицательный
                    floatResultMantissa *= -1;
                }
            }
            // std::cout << baseLoged << " " << logedResultMantissa << " " << fractionOfLogedResultMantissa << " " << floatResultMantissa << " " << rawFinalExp << std::endl;

            Number finalNumber = makeNumber(floatResultMantissa, rawFinalExp);  // (4.929503, 18) -> Number {4929503, 12}
            Number normalizedNum = getNormalized(finalNumber);
            return normalizedNum;
        }
    
        static Number get(const Number& base, const Number& exp) {
            // Это должен быть алгоритм, используемый в крайнем случае
            return Exponent::calculationWithLogarithms(base, exp);
        }
    };

    // Натуральный логарифм
    struct Log : Base {
        std::shared_ptr<Base> arg;

        // Log(Number val) {  // Т.к. мы не можем точно вычислить любой логарифм (например log(3)), то мы его сохраним в изнчальном виде
        //     arg = std::make_shared<NumberWrapper>(val);
        // }

        Log(std::shared_ptr<Base> val) {
            arg = val;
        }
        
        std::shared_ptr<Base> formed();

        Number forcedCalc() const override {
            return get(arg->forcedCalc());
        }

        void printTree(int indent = 0) {
            std::cout << std::string(indent*2, ' ') << "LOG" << std::endl;
            arg->printTree(indent+2);
        }

    private:
        static Number get(const Number& arg) {
            return getNaturalLogged(arg);
        }

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
                auto newLogArg = Mult(leftLog->arg, rightLog->arg, false).formed();
                auto result = Log(newLogArg).formed();
                return result;
                // return std::make_shared<Log>(newLogArg)->formed();
            }
        }

        return std::make_shared<Add>(this->arg1, this->arg2, this->subtractionMode);
    }

    // Эта функция несиметричная: a*10^b преобразуется, а (10^b)*a уже нет
    inline std::shared_ptr<Base> Mult::formed() {
        if (NumberWrapper* leftNum = dynamic_cast<NumberWrapper*>(arg1.get())) {
            if (NumberWrapper* rightNum = dynamic_cast<NumberWrapper*>(arg2.get())) {
                if (!this->divisionMode) {  // Умножение всегда происходит без потери точности
                    auto result = Mult::get(leftNum->num, rightNum->num, false);
                    return std::make_shared<NumberWrapper>(result);
                }
                // Деление
                double testResult = static_cast<double>(leftNum->mantissa) / rightNum->mantissa;
                if (isInt(testResult)) {  // rightNum делится на leftNum без остатка
                    auto result = Mult::get(leftNum->num, rightNum->num, true);
                    return std::make_shared<NumberWrapper>(result);
                }
            }

            // Есть выражение a * b/c, так как b не делится на c (иначе бы b/c схлопнулось), то мы пытаемся перегруппировать выражение: b * a/c,
            // чтобы, возможно, a/c упростилось: 3 * (2/3) => 2 * (3/3) => 2 * 1 => 2
            if (Mult* rightDiv = dynamic_cast<Mult*>(arg2.get())) {
                if (rightDiv->divisionMode) {
                    auto newDivision = Mult(arg1, rightDiv->arg2, true)  // Мы используем arg1, а не leftNum, так как у Mult нет конструктора Mult(Base, shared_ptr<Base>)
                                                                       .formed();  // Пытаемся упростить новое выражение
                    auto result = Mult(rightDiv->arg1, newDivision, this->divisionMode).formed();
                    return result;
                }
            }
            if (Exponent* rightExp = dynamic_cast<Exponent*>(arg2.get())) {
                // Выражение вида (mant1*10^exp1)*10^(mant2*10^exp2) = mant1 * 10^(exp1 + mant2*10^exp2)
                if (equals(rightExp->base->forcedCalc(), Number {10, 0})) {
                    Number expNum = rightExp->exp->forcedCalc();
                    // Без потери точности мы можем посчитать это выражение только при exp2 >= 0 (когда exp1 + mant2*10^exp2 - целое число)
                    if (expNum.exponent >= 0) {
                        long expandedExp = expNum.mantissa * pow(10, expNum.exponent);
                        auto result = Number {leftNum->mantissa, leftNum->exponent + expandedExp};
                        return std::make_shared<NumberWrapper>(result);
                    }
                }
            }
        }

        // Свойство: a^b * a^c = a^(b+c)
        if (Exponent* leftExp = dynamic_cast<Exponent*>(arg1.get())) {
            if (Exponent* rightExp = dynamic_cast<Exponent*>(arg2.get())) {
                if (equals(leftExp->base, rightExp->base)) {
                    auto result = Exponent(
                        leftExp->base,
                        Add(leftExp->exp, rightExp->exp, false).formed()
                    ).formed();
                    return result;
                }
            }
        }

        return std::make_shared<Mult>(this->arg1, this->arg2, this->divisionMode);
    }

    inline std::shared_ptr<Base> Exponent::formed() {
        if (NumberWrapper* baseNum = dynamic_cast<NumberWrapper*>(base.get())) {
            if (NumberWrapper* expNum = dynamic_cast<NumberWrapper*>(exp.get())) {
                if (this->isPurelyComputable()) {
                    auto result = Exponent::get(baseNum->num, expNum->num);
                    return std::make_shared<NumberWrapper>(result);
                }
            }
        }

        return std::make_shared<Exponent>(this->base, this->exp);
    }

    inline std::shared_ptr<Base> Log::formed() {
        if (NumberWrapper* argNum = dynamic_cast<NumberWrapper*>(arg.get())) {
            // ln(1) = 0
            if (argNum->mantissa == 1 && argNum->exponent == 0) {
                return std::make_shared<NumberWrapper>(0, 0);
            }
            // auto result = Log::get(argNum->num);
            // return std::make_shared<NumberWrapper>(result);
        }

        // ln(e) = 1
        if (ConstWrapper* argConst = dynamic_cast<ConstWrapper*>(arg.get())) {
            if (argConst->constType == Const::E) {
                return std::make_shared<NumberWrapper>(1, 0);
            }
        }
        
        // Свойство: log(a^b) = b*log(a)
        if (Exponent* argExp = dynamic_cast<Exponent*>(arg.get())) {
            auto result = Mult(argExp->exp, Log(argExp->base).formed(), false).formed();
            return result;
        }

        return std::make_shared<Log>(this->arg);
    }

    inline void printTree(std::shared_ptr<Base> tree) {
        tree->printTree(0);
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