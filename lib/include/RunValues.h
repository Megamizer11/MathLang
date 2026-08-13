#pragma once

// #include <string>
// #include <vector>
#include <iostream>
// #include <memory>
#include <type_traits>

#include "AST.h"
#include "mpark/variant.hpp"
#include "exceptions.h"

struct NumberValue {
    float value;
};

using Value = mpark::variant<
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

inline std::string as_string(const Value& val) {  /// Должна конвертировать абсолютно любой Value в строку
    if (const NumberValue* valInt = mpark::get_if<NumberValue>(&val)) {
        return std::to_string(valInt->value);
    }
    // if (const NumberValue* valInt = val.get_if<NumberValue>()) {
    //     return std::to_string(valInt->value);
    // }
    throw RunnerException("RunValues > as_string error (that must never happen): this value can\'t be string");
}

inline std::ostream& operator<<(std::ostream& os, const NumberValue& val) {
    return os << val.value;
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

// class VariableScope {  // Взято из моего предыдущего языка, где были уровни вложенности, но в этом языке это избыточно
// public:
//     VariableScope* parent = nullptr;
//     std::vector<Variable> vars;

//     VariableScope() {}

//     void setLocalScope(VariableScope& localScope) {
//         localScope.parent = this;
//     }

//     void clearLocalScope() {
//     }

//     void printAllVars(int indent = 0) {
//         if (parent) {
//             parent->printAllVars();
//         }
//         std::cout << std::string(indent, ' ') << "VARS IN UNNESTED LVL " << ": " << std::endl;
//         for (const Variable& var : this->vars) {
//             std::cout << std::string(indent + 2, ' ') << "name: " << var.name << ", value: " << var.value << std::endl;
//         }
//         std::cout << std::string(indent, ' ') << "PRINT VARS END" << std::endl;
//     }

//     const Variable* getByName(std::string name, const Token errorTooltipToken) {
//         for (const Variable& var : this->vars) {
//             if (var.name == name) {
//                 return &var;  // Условие выхода из рекурсии
//             }
//         }
//         if (this->parent != 0)
//             return this->parent->getByName(name, errorTooltipToken);
        
//         throw RunnerException("VariableScope::getByName error: variable/function {got_literal} was not declarated at {pos}", errorTooltipToken);
//     }

//     Variable* getMutableByName(std::string name, const Token errorTooltipToken) {
//         for (Variable& var : this->vars) {
//             if (var.name == name) {
//                 return &var;  // Условие выхода из рекурсии
//             }
//         }
//         if (this->parent != 0)
//             return this->parent->getMutableByName(name, errorTooltipToken);
        
//         throw RunnerException("VariableScope::getMutableByName error: variable/function {got_literal} was not declarated at {pos}", errorTooltipToken);
//     }
    
//     void addVar(std::string name, Value value) {  // Если переменной ещё нет
//         Variable var = Variable {name, value};
//         vars.push_back(var);
//     }
    
//     void addVar(Variable var) {  // Если переменной ещё нет
//         vars.push_back(var);
//     }

//     // const Token&
//     bool setValue(std::string name, Value value, const Token errorTooltipToken) {  // Если переменная уже есть
//         for (Variable& var : this->vars) {
//             if (var.name == name) {
//                 var.value = value;
//                 return true;  // Условие выхода из рекурсии
//             }
//         }
//         if (parent)
//             return parent->setValue(name, value, errorTooltipToken);
        
//         throw RunnerException("VariableScope::setValue error: variable/function {got_literal} was not declarated at {pos}", errorTooltipToken);
//     }
// };