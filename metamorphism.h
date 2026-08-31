#pragma once
#include "liberaries.h"
using namespace std;
// Function to shuffle and execute the function pointers

std::string getMetamorphicFunc() {

    string funcCode =  "using VoidFunction = void (*)();\n"
        "void executeRandomOrder(std::vector<VoidFunction>&funcs) {\n"
        "    std::random_device rd;\n"
        "    std::mt19937 g(rd());\n"
        "    std::shuffle(funcs.begin(), funcs.end(), g);\n"
        "    for (auto& func : funcs) {\n"
        "        func();\n"
        "    }\n"
        "}\n";
    return funcCode;
}


std::vector<std::string> extractFunctionNames(const std::string& code) {
    std::vector<std::string> functionNames;
    std::istringstream codeStream(code);
    std::string line;
    bool isMetamorphismComment = false;
    std:: regex functionRegex(R"((\b[\w:<>\*]+)\s+(\b[\w:]+)\s*\(([^)]*)\))");


    while (std::getline(codeStream, line)) {
        if (line.find("//metamorphism") != std::string::npos) {
            isMetamorphismComment = true;
        }
        else if (isMetamorphismComment) {
            std::smatch match;
            if (std::regex_search(line, match, functionRegex)) {
                functionNames.push_back(match[2]); // Capture the function name
            }
            isMetamorphismComment = false; // Reset the flag
        }
    }

    return functionNames;
}

// Function to apply metamorphism to the code
std::string applyMetamorphism(const std::string& code, const std::vector<std::string>& functionNames, bool flag) {
    std::string func_code = getMetamorphicFunc();
    // Step 1: Insert `func_code` after the last `#include`.
    std::regex includeRegex(R"(#include\s*<.*>)");
    std::string modifiedCode = code;
    size_t lastIncludePos = std::string::npos;
    size_t lastIncludeLength = 0;

    for (auto it = std::sregex_iterator(code.begin(), code.end(), includeRegex); it != std::sregex_iterator(); ++it) {
        lastIncludePos = it->position();
        lastIncludeLength = it->length();
    }
    if (flag == false) {
        if (lastIncludePos != std::string::npos) {
            modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n" + func_code);
        }
    }
    // Check if <vector> is already included
    std::regex vectorIncludeRegex(R"(#include\s*<\s*vector\s*>)");
    if (!std::regex_search(modifiedCode, vectorIncludeRegex)) {
        // Add #include <vector> after the last include directive or at the start
        if (lastIncludePos != std::string::npos) {
            // Insert after the last include directive
            modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n#include <vector>");
        }
        else {
            // No #include directives found; add at the top
            modifiedCode = "#include <vector>\n" + modifiedCode;
        }
    }

    // Check if <algorithm> is already included
    std::regex algorithmIncludeRegex(R"(#include\s*<\s*algorithm\s*>)");
    if (!std::regex_search(modifiedCode, algorithmIncludeRegex)) {
        // Add #include <algorithm> after the last include directive or at the start
        if (lastIncludePos != std::string::npos) {
            // Insert after the last include directive
            modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n#include <algorithm>");
        }
        else {
            // No #include directives found; add at the top
            modifiedCode = "#include <algorithm>\n" + modifiedCode;
        }
    }

    // Check if <random> is already included
    std::regex randomIncludeRegex(R"(#include\s*<\s*random\s*>)");
    if (!std::regex_search(modifiedCode, randomIncludeRegex)) {
        // Add #include <algorithm> after the last include directive or at the start
        if (lastIncludePos != std::string::npos) {
            // Insert after the last include directive
            modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n#include <random>");
        }
        else {
            // No #include directives found; add at the top
            modifiedCode = "#include <random>\n" + modifiedCode;
        }
    }

    // Step 2: Locate `int main()` and insert the functionNames vector and call.
    //size_t mainPos = modifiedCode.find("int main");
    std::regex mainRegex(R"(\bint\s+main\s*\(\s*\))");
    std::smatch match;
    
    //if (mainPos != std::string::npos) {
    if (std::regex_search(modifiedCode, match, mainRegex)) {
        // Find the position of `int main`
        size_t mainPos = match.position();
        // Find the opening brace `{` after `int main()`
        size_t bracePos = modifiedCode.find('{', mainPos);
        if (bracePos != std::string::npos) {
            // Build the functionNames code
            std::string funcNamesCode = "std::vector<VoidFunction> functionNames = { ";
            for (size_t i = 0; i < functionNames.size(); ++i) {
                funcNamesCode += functionNames[i];
                if (i != functionNames.size() - 1) {
                    funcNamesCode += ", ";
                }
            }
            funcNamesCode += " };\n    executeRandomOrder(functionNames);\n";

            // Insert after the opening brace
            modifiedCode.insert(bracePos + 1, "\n    " + funcNamesCode);
        }
    }

    return modifiedCode;
}