// #include <iostream>

// #include "tokens.h"
// #include "AST.h"

// // Если печатание резко обрывается, то, скорее всего print принял nullptr

// // Поля: Prime - string, int, bool; Expanding - TokenType, Token, Pointer - все ноды

// #define GET_STR_INDENT(shift) std::string(indent + (shift), ' ')

// #define END_NODE_PRINT() std::cout << "\n" << GET_STR_INDENT(0) << "}";

// #define PRINT_NODE_PROLOGUE(class_name) \
//     std::cout << "{\n"; \
//     // std::cout << std::string(indent + 2, ' ') << #class_name"()\n";

//     // std::string decor = ("\""decltype(field_name)"\"" == "int") ? "" : "\"";
// #define PRIME_MID_FIELD(field_name) \
//     {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
//     std::cout << GET_STR_INDENT(2) << "\""#field_name"\"" << ": " << decor << field_name << decor; \
//     std::cout << ", \n";}

// #define PRIME_END_FIELD(field_name) \
//     {std::string decor = (std::is_same<decltype(field_name), int>::value) ? "" : "\""; \
//     std::cout << GET_STR_INDENT(2) << "\""#field_name"\"" << ": " << decor << field_name << decor; \
//     END_NODE_PRINT()}

// #define EXPANDING_MID_FIELD(field_name) \
//     {std::cout << GET_STR_INDENT(2) << "\""#field_name"\"" << ": "; \
//     field_name.print(indent+2); \
//     std::cout << ", \n";}

// #define EXPANDING_END_FIELD(field_name) \
//     {std::cout << GET_STR_INDENT(2) << "\""#field_name"\"" << ": "; \
//     field_name.print(indent+2); \
//     END_NODE_PRINT()}

// #define POINTER_MID_FIELD(field_name) \
//     {std::cout << GET_STR_INDENT(2) << "\""#field_name"\"" << ": "; \
//     field_name->print(indent+2); \
//     std::cout << ", \n";}

// #define POINTER_END_FIELD(field_name) \
//     {std::cout << GET_STR_INDENT(2) << "\""#field_name"\"" << ": "; \
//     field_name->print(indent+2); \
//     END_NODE_PRINT()}


// void TokenType::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(TokenType_)
//     PRIME_END_FIELD(name)
// }

// void Token::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(Token)
//     PRIME_MID_FIELD(type.name)
//     PRIME_END_FIELD(literal)
// }

// void BaseNode::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(BaseNode)
// }

// void RootNode::print(int indent = 0) {
//     std::cout << "RootNode(): \n[\n";
//     for (const auto& elem : instructions) {
//         std::cout << GET_STR_INDENT(2);
//         elem->print(indent+2);
//         if (elem != instructions.back())
//             std::cout << ", \n";
//     }
//     std::cout << "\n]\n";
// }

// void ExpressionNode::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(ExpressionNode)
// }

// void NumberNode::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(NumberNode)
//     EXPANDING_END_FIELD(numberToken)
// }

// void VariableNode::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(VariableNode)
//     EXPANDING_END_FIELD(varToken)
// }

// void BinNode::print(int indent = 0) {
//     PRINT_NODE_PROLOGUE(BinNode)
//     EXPANDING_MID_FIELD(operToken)
//     POINTER_MID_FIELD(left)
//     POINTER_END_FIELD(right)
//     // POINTER_MID_FIELD(left)
//     // POINTER_END_FIELD(right)
// }

// #undef GET_STR_INDENT
// #undef END_NODE_PRINT
// #undef PRINT_NODE_PROLOGUE
// #undef PRIME_MID_FIELD
// #undef PRIME_END_FIELD
// #undef EXPANDING_MID_FIELD
// #undef EXPANDING_END_FIELD
// #undef POINTER_MID_FIELD
// #undef POINTER_END_FIELD

#include <iostream>
#include <iomanip>

#include "tokens.h"
#include "AST.h"

#define GET_STR_INDENT(shift) std::string(indent + (shift), ' ')

void TokenType::print(int indent = 0) {
    std::cout << GET_STR_INDENT(0) << this->name << std::endl;
}

void Token::print(int indent = 0) {
    // std::cout << GET_STR_INDENT(0) << "╚ " << this->literal << std::endl;  // Так виднее, к какому символу относятся другие символы
    std::cout << GET_STR_INDENT(0) << this->type.name << ": " << this->literal << std::endl;
    // std::cout << this->fLine << ":" << this->fIndex << GET_STR_INDENT(0) << this->type.name << ": " << this->literal << std::endl;
    // std::cout << this->literal << std::endl;
    // std::cout << " " << this->literal;
}

void BaseNode::print(int indent = 0) {
    std::cout << GET_STR_INDENT(0) << "BASE_NODE_ERR";
}

void RootNode::print(int indent = 0) {
    std::cout << "RootNode(): [\n";
    for (const auto& elem : instructions) {
        elem->print(indent);
        std::cout << std::endl;
    }
    std::cout << "]\n";
}

void ExpressionNode::print(int indent = 0) {
    std::cout << GET_STR_INDENT(0) << "EXPRESSION_NODE_ERR";
}

void NumberNode::print(int indent = 0) {
    numberToken.print(indent+2);
}

void VariableNode::print(int indent = 0) {
    varToken.print(indent+2);
}

void BinNode::print(int indent = 0) {
    // operToken.print(indent+2);
    // left->print(indent+2);
    left->print(indent+2);      // В таком виде можно последовательно видеть дерево
    operToken.print(indent+2);  //
    right->print(indent+2);
}

void SideEffectFuncNode::print(int indent = 0) {
    operToken.print(indent+2);
    arg->print(indent+2);
}

#undef GET_STR_INDENT