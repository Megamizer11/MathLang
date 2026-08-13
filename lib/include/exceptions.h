#pragma once

// #include <vector>
#include <memory>
#include <cstdarg>
#include <string>

#include "tokens.h"

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

    ParserException(std::string msg, const Token& token, int formatCount, ...) {
        message = msg;
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
        
        va_list formatList;
        va_start(formatList, formatCount);
        for (int i = 0; i < formatCount ; i++) {
            std::string repl = "{" + std::to_string(i) + "}";
            std::string arg = va_arg(formatList, std::string);
            if (message.find(repl) != std::string::npos)
                message.replace(message.find(repl), repl.length(), arg);
        }
        va_end(formatList);
    }

    // const char* what();
    virtual const char* what() const noexcept override {
        // std::string msgCopy = message;
        // msgCopy.replace(msgCopy.find("{pos}"), 5, "0:0");
        return message.c_str();
    }
};


class RunnerException : public std::exception {
private:
    std::string message;
public:
    RunnerException(std::string msg) {
        message = msg;
    }

    RunnerException(std::string msg, const Token& token) {
        message = msg;
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
    }

    // RunnerException(std::string msg, int formatCount, ...) {
    //     va_list formatList;
    //     va_start(formatList, formatCount);
    //     for (int i = 0; i < formatCount ; i++) {
    //         std::string repl = "{" + std::to_string(i) + "}";
    //         if (message.find(repl) != std::string::npos)
    //             message.replace(message.find(repl), repl.length(), va_arg(formatCount, std::string));
    //     }
    // }

    RunnerException(std::string msg, const Token& token, int formatCount, ...) {
        message = msg;
        std::string tokenPos = std::to_string(token.fLine) + ":" + std::to_string(token.fIndex);
        if (message.find("{pos}") != std::string::npos)
            message.replace(message.find("{pos}"), 5, tokenPos);
        if (message.find("{got_literal}") != std::string::npos)
            message.replace(message.find("{got_literal}"), 13, "\"" + token.literal + "\"");
        
        va_list formatList;
        va_start(formatList, formatCount);
        for (int i = 0; i < formatCount ; i++) {
            std::string repl = "{" + std::to_string(i) + "}";
            std::string arg = va_arg(formatList, std::string);
            if (message.find(repl) != std::string::npos)
                message.replace(message.find(repl), repl.length(), arg);
        }
        va_end(formatList);
    }

    virtual const char* what() const noexcept override {
        return message.c_str();
    }
};