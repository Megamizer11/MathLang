#pragma once

// #include <vector>
#include <memory>
#include <cstdarg>
#include <string>
#include <exception>
#include <iostream>
#include <tuple>

#include "tokens.h"

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
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};