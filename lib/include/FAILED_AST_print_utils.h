#include <iostream>

#include "tokens.h"
#include "AST.h"

// Если печатание резко обрывается, то, скорее всего print принял nullptr

// Суффикс N7A1D нужен, чтобы не было случайного конфликта имён
// Поля: Prime - string, int, bool; Expanding - TokenType, Token, Pointer - все ноды

#define INDENT_N7A1D(shift) std::string(indent + (shift), ' ')

#define END_PRINT() std::cout << "\n" << INDENT_N7A1D(0) << "}";

// PROLOG
#define CLASS_NAME_N7A1D(class_name) \
    std::cout << "{\n"; \
    std::cout << std::string(indent + 2, ' ') << #class_name"()\n";

    // std::string decor = ("\""decltype(field_name)"\"" == "int") ? "" : "\"";
#define PRIME_MID_FIELD(field_name) \
    {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
    std::cout << INDENT_N7A1D(2) << "\""#field_name"\"" << ": " << decor << field_name << decor; \
    std::cout << ", \n";}

#define PRIME_END_FIELD(field_name) \
    {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
    std::cout << INDENT_N7A1D(2) << "\""#field_name"\"" << ": " << decor << field_name << decor; \
    END_PRINT()}

#define EXPANDING_MID_FIELD(field_name) \
    {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
    std::cout << INDENT_N7A1D(2) << "\""#field_name"\"" << ": "; \
    print(field_name, indent+2); \
    std::cout << ", \n";}

#define EXPANDING_END_FIELD(field_name) \
    {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
    std::cout << INDENT_N7A1D(2) << "\""#field_name"\"" << ": "; \
    print(field_name, indent+2); \
    END_PRINT()}

#define POINTER_MID_FIELD(field_name) \
    {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
    std::cout << INDENT_N7A1D(2) << "\""#field_name"\"" << ": "; \
    print(field_name.get(), indent+2); \
    std::cout << ", \n";}

#define POINTER_END_FIELD(field_name) \
    {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
    std::cout << INDENT_N7A1D(2) << "\""#field_name"\"" << ": "; \
    print(field_name.get(), indent+2); \
    END_PRINT()}


void print(const TokenType* tokenType, int indent = 0) {  // nodePrint
    CLASS_NAME_N7A1D(TokenType)
    PRIME_END_FIELD(tokenType->name)
}

void print(const Token* token, int indent = 0) {
    CLASS_NAME_N7A1D(Token)
    EXPANDING_END_FIELD(&token->type)
    PRIME_END_FIELD(token->literal)
}

void print(const BaseNode* node, int indent = 0) {
    CLASS_NAME_N7A1D(BaseNode)
    std::cout << "BASE" << std::endl;
    if (const NumberNode* realNode = dynamic_cast<const NumberNode*>(node)) {
        std::cout << "NUM" << std::endl;
        print(realNode, indent);
    } else if (const VariableNode* realNode = dynamic_cast<const VariableNode*>(node)) {
        std::cout << "VAR" << std::endl;
        print(realNode, indent);
    } else if (const BinNode* realNode = dynamic_cast<const BinNode*>(node)) {
        std::cout << "BIN" << std::endl;
        print(static_cast<const BinNode*>(realNode), indent);
    }
}

// void print(const BaseNode* node, int indent = 0) {
//     if (const BinNode* realNode = dynamic_cast<const BinNode*>(node)) {
//         print(realNode, indent);
//     }
// }

void print(const RootNode* node, int indent = 0) {
    std::cout << "RootNode(): \n[\n";
    for (const auto& elem : node->instructions) {
        std::cout << INDENT_N7A1D(2);
        std::cout << "LOOP" << std::endl;
        print(elem.get(), indent+2);
        if (elem != node->instructions.back())
            std::cout << ", \n";
    }
    std::cout << "\n]\n";
}

void print(const ExpressionNode* node, int indent = 0) {
    CLASS_NAME_N7A1D(ExpressionNode)
    std::cout << "EXPR" << std::endl;
    if (const NumberNode* realNode = dynamic_cast<const NumberNode*>(node)) {
        print(realNode, indent);
    } else if (const VariableNode* realNode = dynamic_cast<const VariableNode*>(node)) {
        print(realNode, indent);
    } else if (const BinNode* realNode = dynamic_cast<const BinNode*>(node)) {
        print(realNode, indent);
    }
}

void print(const NumberNode* node, int indent = 0) {
    CLASS_NAME_N7A1D(NumberNode)
    EXPANDING_END_FIELD(&node->numberToken)
}

void print(const VariableNode* node, int indent = 0) {
    CLASS_NAME_N7A1D(VariableNode)
    EXPANDING_END_FIELD(&node->varToken)
}

void print(const BinNode* node, int indent = 0) {
    std::cout << "BINNNNN" << std::endl;
    CLASS_NAME_N7A1D(BinNode)
    EXPANDING_MID_FIELD(&node->operToken)
    // POINTER_MID_FIELD(node->left)
    // POINTER_END_FIELD(node->right)
    // print(node->left.get());
    // print(node->right.get());
}