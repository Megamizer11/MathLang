#pragma once

#include <memory>
#include <cstdarg>
#include <string>

// #include "tokens.h"
#include "AST.h"
#include "RunValues.h"

class Runner {
private:
    VariableScope varScope {};
    RootNode& rootNode;
public:
    Runner(RootNode&);
    VariableScope run();
};