#pragma once

// #include <vector>
#include <memory>
#include <cstdarg>
#include <string>
#include <exception>
#include <iostream>
#include <tuple>

#include "tokens.h"

// // ВАЖНО! Если передавать в третий констуктор не строки ParserException("Err at {0}", token, 1, pos) (pos - число), то программа упадёт с Undefined behaviour
// class ParserException : public std::exception {
// private:
//     std::string message;
// public:
//     // ParserException(const char* msg) : message(msg) {}
//     ParserException(std::string msg) {
//         message = msg;
//     }

//     ParserException(std::string msg, const Token& token) {
//         message = msg;
//         std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
//         if (message.find("{pos}") != std::string::npos)
//             message.replace(message.find("{pos}"), 5, tokenPos);
//         if (message.find("{got_literal}") != std::string::npos)
//             message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
//     }

//     ParserException(std::string msg, const Token& token, int formatCount, ...) {
//         message = msg;
//         std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
//         if (message.find("{pos}") != std::string::npos)
//             message.replace(message.find("{pos}"), 5, tokenPos);
//         if (message.find("{got_literal}") != std::string::npos)
//             message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
        
//         va_list formatList;
//         va_start(formatList, formatCount);
//         for (int i = 0; i < formatCount ; i++) {
//             std::string repl = "{" + std::to_string(i) + "}";
//             std::string arg = va_arg(formatList, std::string);
//             if (message.find(repl) != std::string::npos)
//                 message.replace(message.find(repl), repl.length(), arg);
//         }
//         va_end(formatList);
//     }

//     virtual const char* what() const noexcept override {
//         return message.c_str();
//     }
// };


// // ВАЖНО! Если передавать в третий констуктор не строки RunnerException("Err at {0}", token, 1, pos) (pos - число), то программа упадёт с Undefined behaviour
// class RunnerException : public std::runtime_error {
// private:
//     std::string message = "RAW_ERR";
// public:
//     // RunnerException(std::string msg) {message = msg;}
//     RunnerException(const std::string& msg):
//         std::runtime_error(msg) {}
//     // RunnerException(const std::string& text) : std::runtime_error(text) {}

//     // RunnerException(std::string msg, const Token& token): std::runtime_error(formatMsg(msg, token)) {}
//     // RunnerException(std::string msg, const Token& token, int formatCount, ...): std::runtime_error(formatMsg(msg, token)) {}

//     RunnerException(std::string msg, const Token& token): std::runtime_error("RAW") {
//         std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
//         message = msg;
//         if (message.find("{pos}") != std::string::npos)
//             message.replace(message.find("{pos}"), 5, tokenPos);
//         if (message.find("{got_literal}") != std::string::npos)
//             message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
//     }

//     RunnerException(std::string msg, const Token& token, int formatCount, ...): std::runtime_error("RAW") {
//         std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
//         message = msg;
//         if (message.find("{pos}") != std::string::npos)
//             message.replace(message.find("{pos}"), 5, tokenPos);
//         if (message.find("{got_literal}") != std::string::npos)
//             message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
        
//         va_list formatList;
//         va_start(formatList, formatCount);
//         for (int i = 0; i < formatCount ; i++) {
//             std::string repl = "{" + std::to_string(i) + "}";
//             std::string arg = va_arg(formatList, std::string);
//             // const char* arg_ptr = va_arg(formatList, const char*);
//             // std::string arg = std::string(arg_ptr);
//             if (message.find(repl) != std::string::npos)
//                 message.replace(message.find(repl), repl.length(), arg);
//         }
//         va_end(formatList);
//     }

//     const char* what() const noexcept override {
//         return message.c_str();
//     }
// };

template<typename T>
inline std::string safe_to_string(T arg) {
    return std::to_string(arg);
}

inline std::string safe_to_string(const std::string& arg) { return arg; }

inline std::string safe_to_string(const char* arg) { return arg; }


class ParserException : public std::exception {
private:
    std::string message;
public:
    // ParserException(const char* msg) : message(msg) {}
    ParserException(std::string msg) {
        message = msg;
    }

    ParserException(std::string msg, const Token& token) {
        message = msg;
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
    }

    template<typename... Args>
    ParserException(std::string msg, const Token& token, const Args&... args) {
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        message = msg;
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
        
        constexpr int argsLen = sizeof...(Args);
        std::vector<std::string> vecOfArgs = {safe_to_string(args)...};
        for (int i = 0; i < argsLen; i++) {
            std::string repl = "{" + std::to_string(i) + "}";
            std::string arg = vecOfArgs.at(i);
            if (message.find(repl) != std::string::npos)
                message.replace(message.find(repl), repl.length(), arg);
        }
    }

    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};

class RunnerException : public std::runtime_error {
private:
    std::string message;
public:
    RunnerException(const std::string& msg):
        std::runtime_error(msg) {}

    RunnerException(std::string msg, const Token& token): std::runtime_error("RAW_MSG") {
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        message = msg;
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
    }

    template<typename... Args>
    RunnerException(std::string msg, const Token& token, const Args&... args): std::runtime_error("RAW_MSG") {
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        message = msg;
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
        
        constexpr int argsLen = sizeof...(Args);
        std::vector<std::string> vecOfArgs = {safe_to_string(args)...};
        for (int i = 0; i < argsLen; i++) {
            std::string repl = "{" + std::to_string(i) + "}";
            std::string arg = vecOfArgs.at(i);
            if (message.find(repl) != std::string::npos)
                message.replace(message.find(repl), repl.length(), arg);
        }
        // std::tuple<Args...> tupleOfArgs = std::make_tuple(std::forward<Args>(args)...);
        // auto tupleOfArgs = std::tuple<Args...>(args...);
        // std::apply([](auto&&... args) {((
        //     std::cout << args << std::endl;
        // ), ...);}, tupleOfArgs);
        // for (int i = 0; i < argsLen ; i++) {
        //     std::string repl = "{" + std::to_string(i) + "}";
        //     // std::string arg = std::get<i>(tupleOfArgs);
        //     // if (message.find(repl) != std::string::npos)
        //     //     message.replace(message.find(repl), repl.length(), arg);
        // }
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};