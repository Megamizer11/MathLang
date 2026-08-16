#pragma once

#include <utility>
#include <string>

inline long long string_to_integral(std::string str) {
    return std::stoll(str);
}

// std::pair<long long, long> getMantissaAndExponentFromLiteral(std::string literal) {
//     bool isInt = literal.find(".") == std::string::npos;
//     long long mantissa;
//     long long exponent = 0;
//     std::string strMantissa = literal;
//     if (isInt) {
//         // mantissa = std::stoi(strMantissa);
//     } else {
//         strMantissa.erase(literal.find("."), 1);
//         // mantissa = std::stoi(strMantissa);
//     }
//     // for (int i = strMantissa.length(); i <= 0; i--) {}
//     int i = strMantissa.length() - 1;
//     char ch = strMantissa[strMantissa.length() - 1];
//     while (strMantissa[i] == '0') {
//         strMantissa.pop_back();
//         i--;
//         exponent++;
//     }
//     mantissa = std::stoi(strMantissa);
//     return std::make_pair(mantissa, exponent);
// }

// У числа 12000 мантисса=12, экспонента=3, у 12000.001 мантисса=12000001, экспонента=-3, а незначащие нули (1.10 и 01.1) не влияют на число
inline std::pair<long long, long> getMantissaAndExponentFromLiteral(std::string literal) {
    if (literal == "0")
        return std::make_pair(0, 0);
    bool isInt = literal.find(".") == std::string::npos;
    long long mantissa = 0;
    long long exponent = 0;
    std::string strMantissa = literal;
    if (isInt) {
        int i = strMantissa.length() - 1;
        char ch = strMantissa[strMantissa.length() - 1];
        while (strMantissa[i] == '0') {
            strMantissa.pop_back();
            i--;
            exponent++;
        }
    } else {
        int i = strMantissa.length() - 1;
        char ch = strMantissa[strMantissa.length() - 1];
        while (strMantissa[i] == '0') {  // Просто убираем лишние нули у дроби (у 13.20 strMantissa="1320" => strMantissa="132")
            strMantissa.pop_back();
            i--;
        }

        int dotIdx = strMantissa.find(".");
        strMantissa.erase(dotIdx, 1);
        exponent = - (strMantissa.length() - dotIdx);
    }
    mantissa = string_to_integral(strMantissa);
    return std::make_pair(mantissa, exponent);
}

// std::pair<long long, long> getMantissaAndExponentFromLiteral(std::string literal) {
//     bool isInt = literal.find(".") == std::string::npos;
//     long long mantissa;
//     long long exponent = 0;
//     // for (int i = 0; i < literal.length(); i++) {}
//     bool curIsDecimal = !isInt;
//     char ch;
//     for (int i = literal.length() - 1; i <= 0; i++) {
//         ch = literal[i];
//         if (ch == '.')
//             curIsDecimal = false;  // Т.к. по циклу мы идём в обратную сторону
//     }
//     return std::make_pair(mantissa, exponent);
// }

inline std::string getLiteralFromMantissaAndExponent(long long mantissa, long exponent) {
    std::string strMantissa = std::to_string(mantissa);
    bool isNegative = mantissa < 0;
    if (isNegative)
        strMantissa.erase(0, 1);
    
    if (exponent > 0) {
        strMantissa += std::string(exponent, '0');
    }
    if (exponent < 0) {
        size_t strMantissaLen = strMantissa.length();
        if (-exponent < strMantissaLen) {  // 12345, -1 -> 1234.5
            unsigned long long dotIdx = strMantissaLen - -exponent;
            strMantissa.insert(dotIdx, ".");
        } else if (-exponent > strMantissaLen) {  // 12345, -6 -> 0.012345
            unsigned long long additionalZeros = -exponent - strMantissaLen;
            strMantissa.insert(0, std::string(additionalZeros, '0'));
            strMantissa = "0." + strMantissa;
        } else {  // 12345, -5 -> 0.12345
            strMantissa = "0." + strMantissa;
        }
    }

    if (isNegative)
        strMantissa = "-" + strMantissa;
    return strMantissa;
}