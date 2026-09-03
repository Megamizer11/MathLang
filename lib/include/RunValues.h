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
    // long long mantissa;  // Для числа 12.345 это 12345, для 12000 это 12, мантисса может быть отрицательной, мантисса не может оканчиваться на ноль (исключение - мантисса равна нулю)
    // long exponent;       // Для числа 12.345 это -3, для 12000 это 3, экспонента может быть отрицательной

    std::shared_ptr<symcomp::Base> inner = nullptr;
    // std::shared_ptr<symcomp::Base> unwrappedInner = nullptr;

    NumberValue(const symcomp::Number& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::NumberWrapper>(tree);
    };

    NumberValue(const symcomp::NumberWrapper& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::NumberWrapper>(tree);
    };

    NumberValue(const symcomp::ConstWrapper& tree) {  // По факту внутренний API для символьного вычисления
        inner = std::make_shared<symcomp::ConstWrapper>(tree);
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

    // NumberValue(long long mantissa, long exponent) : mantissa(mantissa), exponent(exponent) {  // Базовый конструктор
    //     inner = std::make_shared<symcomp::NumberWrapper>(mantissa, exponent);
    // };

    NumberValue(long long mantissa, long exponent) {  // Базовый конструктор
        inner = std::make_shared<symcomp::NumberWrapper>(mantissa, exponent);
    };

    bool isInt() {
        return this->inner->forcedCalc().exponent >= 0;
    }

    bool isPositive() {
        return this->inner->forcedCalc().mantissa > 0;
    }
    
    // std::string getAsString() const {
    //     return getLiteralFromMantissaAndExponent(mantissa, exponent);
    // }

    // NumberValue& withUnwrappedInner(std::shared_ptr<symcomp::Base> unwrappedInner) {
    //     this->unwrappedInner = unwrappedInner;
    //     return *this;
    // }
    
    static std::string getNumberAsString(symcomp::Number num) {
        return getLiteralFromMantissaAndExponent(num.mantissa, num.exponent);
    }

    long double asPrimitive() const {
        symcomp::Number answer = this->inner->forcedCalc();
        return answer.mantissa * pow(10, answer.exponent);
    }

    NumberValue getRounded(bool isDown) {  // isDown=true - округление вниз, isDown=false - округление вверх
        return NumberValue(symcomp::getRounded(this->inner->forcedCalc(), isDown));
    }

    std::string getAsString() const {
        if (this->inner != nullptr) {
            auto answer = this->inner->forcedCalc();
            std::string strAnswer = NumberValue::getNumberAsString(answer);
            if (strAnswer.length() > 100000)
                throw RunnerException("NumberValue::getAsString() error: too large value for stringify");
            return strAnswer;
        }
        throw RunnerException("NumberValue::getAsString() error: somehow inner (ptr) == 0");
    }

    NumberValue operator+(const NumberValue& rightNumberValue) {
        auto result = symcomp::Add(
            this->inner,
            rightNumberValue.inner,
            false
        ).formed();
        // return NumberValue(result.formed()).withUnwrappedInner(std::make_shared<symcomp::Add>(result));
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
        return symcomp::getRemainder(
            this->inner->forcedCalc(),
            rightNumberValue.inner->forcedCalc()
        );
    }

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

    // std::shared_ptr<Base> getUnwrappedTree() {}

    void printAsSymbolicTree() {
        this->inner->printTree();
    }

    static NumberValue getAsVariable(std::string varName) {
        return NumberValue(std::make_shared<symcomp::Variable>(varName));
    }

    NumberValue getDerivative() {
        return NumberValue(this->inner->getDerivative());
    }

    // Проверяет, может ли выражение быть вычислено (может, когда в нём нет переменных): 3+2 вычислимое, 3+x невычислимое
    bool isCalculatable() {
        return symcomp::isCalculatable(this->inner);
    }
};

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

inline std::string as_str(const Value& val) {
    if (const NumberValue* valNum = mpark::get_if<NumberValue>(&val)) {
        return valNum->getAsString();
    } else if (const FunctionValue* valFunc = mpark::get_if<FunctionValue>(&val)) {
        return "FUNC CALL VALUE: " + valFunc->name;
    }
}

inline std::ostream& operator<<(std::ostream& os, const NumberValue& val) {
    return os << val.getAsString();
}

inline std::ostream& operator<<(std::ostream& os, const FunctionValue& val) {
    return os << as_str(val);
}

inline std::ostream& operator<<(std::ostream& os, const Value& valueType) {
    mpark::visit([&os](const auto& val) {
        os << val;
    }, valueType);

    return os;
}

// inline NumberValue NumberValue::getDerivative() {
//     return NumberValue(this->inner->getDerivative());
// }

struct Variable {
    std::string name;
    Value value;
};

class VariableScope;

struct LazyVariable {
    std::string name;
    ExpressionNode* lazyValue;
    VariableScope& varScopeForLazyCalc;

    LazyVariable(std::string name, ExpressionNode* lazyValue, VariableScope& varScopeForLazyCalc)
        : name(name), lazyValue(lazyValue), varScopeForLazyCalc(varScopeForLazyCalc) {}
};

class VariableScope {
public:
    using shared_var = std::shared_ptr<Variable>;
    using shared_lazy_var = std::shared_ptr<LazyVariable>;

    std::vector<shared_var> vars;
    std::vector<shared_lazy_var> lazyVars;  // Не используется из-за бага
    std::vector<FunctionValue> functions;

    VariableScope() {}

    void printAllVars(int indent = 0) {
        std::cout << std::string(indent, ' ') << "VARS: " << std::endl;
        for (const shared_var& var : this->vars) {
            std::cout << std::string(indent + 2, ' ') << "name: " << var->name << ", value: " << var->value << std::endl;
        }
        std::cout << std::string(indent, ' ') << "LAZY_VARS: " << std::endl;
        for (const shared_lazy_var& lazyVar : this->lazyVars) {
            std::cout << std::string(indent + 2, ' ') << "name: " << lazyVar->name << ", value: " << std::endl;
            lazyVar->lazyValue->print(0);
        }
        std::cout << std::string(indent, ' ') << "PRINT VARS END" << std::endl;
    }

    bool isVarInScope(std::string name) {
        for (const shared_var& var : this->vars) {
            if (var->name == name)
                return true;
        }
        for (const shared_lazy_var& lazyVar : this->lazyVars) {
            if (lazyVar->name == name)
                return true;
        }
        return false;
    }

    void addVar(Variable var) {
        vars.insert(vars.begin(), std::make_shared<Variable>(var));
    }

    void addVar(std::shared_ptr<Variable> var) {
        // vars.push_back(var);
        vars.insert(vars.begin(), var);  // insert помогает создать иллюзию вложенности
    }

    // void addVar(std::string name, ExpressionNode* lazyValue, VariableScope& varScopeForLazyCalc) {
    //     // vars.push_back(var);
    //     LazyVariable lazyVar = LazyVariable {name, lazyValue, varScopeForLazyCalc};
    //     // lazyVars.insert(vars.begin(), std::make_shared<LazyVariable>(name, lazyValue, varScopeForLazyCalc));
    //     lazyVars.insert(lazyVars.begin(), std::make_shared<LazyVariable>(lazyVar));
    // }

    void addVar(LazyVariable lazyVar) {
        lazyVars.insert(lazyVars.begin(), std::make_shared<LazyVariable>(lazyVar));
    }

    const Variable* getByName(std::string name, const Token errorTooltipToken) {
        for (const shared_var& var : this->vars) {
            if (var->name == name)
                // return &*var;
                return var.get();
        }
        for (const shared_lazy_var& lazyVar : this->lazyVars) {
            if (lazyVar->name == name) {
                std::cout << "NAME " << name << std::endl;
                Value calculated = lazyVar->lazyValue->runNode(lazyVar->varScopeForLazyCalc);
                Variable newVar = Variable {lazyVar->name, calculated};
                addVar(newVar);
                return vars.front().get();
            }
        }
        throw RunnerException("VariableScope::getByName error: variable {got_literal} was not declarated at {pos}", errorTooltipToken);
    }

    bool isFuncInScope(std::string name) {
        for (const FunctionValue& func : this->functions) {
            if (func.name == name)
                return true;
        }
        return false;
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