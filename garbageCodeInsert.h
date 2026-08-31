#pragma once
#include "liberaries.h"

using namespace std;
string garbageTemplate1(string funcName) {
    return "//metamorphism\n"
        "void " + funcName + "() {\n"
        "    std::string input = \"garbage_string_extended\";\n"
        "    std::string result = \"\";\n"
        "    for (size_t i = 0; i < input.size(); ++i) {\n"
        "        if (i % 3 == 0)\n"
        "            result += input[i] + 1;\n"
        "        else if (i % 3 == 1)\n"
        "            result += input[i] - 1;\n"
        "        else\n"
        "            result += input[i];\n"
        "    }\n"
        "    if (!result.empty()) {\n"
        "        for (size_t i = 0; i < result.size() / 2; ++i) {\n"
        "            char temp = result[i];\n"
        "            result[i] = result[result.size() - 1 - i];\n"
        "            result[result.size() - 1 - i] = temp;\n"
        "        }\n"
        "    }\n"
        "    int unique_char_count = 0;\n"
        "    for (size_t i = 0; i < result.size(); ++i) {\n"
        "        bool is_unique = true;\n"
        "        for (size_t j = 0; j < i; ++j) {\n"
        "            if (result[i] == result[j]) {\n"
        "                is_unique = false;\n"
        "                break;\n"
        "            }\n"
        "        }\n"
        "        if (is_unique)\n"
        "            ++unique_char_count;\n"
        "    }\n"
        "}\n";

}

string garbageTemplate2(string funcName) {
    return "//metamorphism\n" 
        "void " + funcName + "() {\n"
        "    int number = 100;\n"
        "    int primes[50] = {0};\n"
        "    int prime_count = 0;\n"
        "    for (int i = 2; i <= number; ++i) {\n"
        "        bool isPrime = true;\n"
        "        for (int j = 2; j * j <= i; ++j) {\n"
        "            if (i % j == 0) {\n"
        "                isPrime = false;\n"
        "                break;\n"
        "            }\n"
        "        }\n"
        "        if (isPrime)\n"
        "            primes[prime_count++] = i;\n"
        "    }\n"
        "    int factorial = 1;\n"
        "    for (int i = 1; i <= prime_count; ++i) {\n"
        "        if (factorial < 5000)\n"
        "            factorial = (factorial * i) % 10000;\n"
        "        else\n"
        "            factorial = factorial % 10000;\n"
        "    }\n"
        "    int prime_factors[50] = {0};\n"
        "    int factor_count = 0;\n"
        "    for (int i = 0; i < prime_count; ++i)\n"
        "        if (factorial % primes[i] != 0)\n"
        "            prime_factors[factor_count++] = primes[i];\n"
        "}\n";

}

string garbageTemplate3(string funcName) {
    return "//metamorphism\n"
        "void " + funcName + "() {\n"
        "    int arr[] = {64, 34, 25, 12, 22, 11, 90, 77, 45};\n"
        "    int n = sizeof(arr) / sizeof(arr[0]);\n"
        "    for (int i = 0; i < n - 1; ++i) {\n"
        "        for (int j = 0; j < n - i - 1; ++j) {\n"
        "            if (arr[j] > arr[j + 1]) {\n"
        "                int temp = arr[j];\n"
        "                arr[j] = arr[j + 1];\n"
        "                arr[j + 1] = temp;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    int unique_elements[50] = {0};\n"
        "    int unique_count = 0;\n"
        "    for (int i = 0; i < n; ++i) {\n"
        "        bool is_unique = true;\n"
        "        for (int j = 0; j < unique_count; ++j) {\n"
        "            if (arr[i] == unique_elements[j]) {\n"
        "                is_unique = false;\n"
        "                break;\n"
        "            }\n"
        "        }\n"
        "        if (is_unique)\n"
        "            unique_elements[unique_count++] = arr[i];\n"
        "    }\n"
        "}\n";

}
string garbageTemplate4(string funcName) {
    return "//metamorphism\n"
        "void " + funcName + "() {\n"
        "    int A[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};\n"
        "    int B[3][3] = {{9, 8, 7}, {6, 5, 4}, {3, 2, 1}};\n"
        "    int C[3][3] = {0};\n"
        "    for (int i = 0; i < 3; ++i) {\n"
        "        for (int j = 0; j < 3; ++j) {\n"
        "            for (int k = 0; k < 3; ++k) {\n"
        "                C[i][j] += A[i][k] * B[k][j];\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "    int flat_matrix[9] = {0};\n"
        "    int index = 0;\n"
        "    for (int i = 0; i < 3; ++i)\n"
        "        for (int j = 0; j < 3; ++j)\n"
        "            flat_matrix[index++] = C[i][j];\n"
        "    for (int i = 0; i < index - 1; ++i) {\n"
        "        for (int j = 0; j < index - i - 1; ++j) {\n"
        "            if (flat_matrix[j] > flat_matrix[j + 1]) {\n"
        "                int temp = flat_matrix[j];\n"
        "                flat_matrix[j] = flat_matrix[j + 1];\n"
        "                flat_matrix[j + 1] = temp;\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n";

}

string garbageTemplate5(string funcName) {
    return "//metamorphism\n"
        "void " + funcName + "() {\n"
        "    int x = 0, y = 0;\n"
        "    int steps = 50;\n"
        "    int path[100][2]; // This stores pairs of coordinates\n"
        "    int pathCount = 0;\n"
        "    unsigned int seed = 123456; // Manual seed for srand()\n"
        "    for (int i = 0; i < steps; ++i) {\n"
        "        int direction = (seed % 4); // Generates a direction manually\n"
        "        seed = seed * 1103515245 + 12345; // Linear congruential generator\n"
        "        switch (direction) {\n"
        "            case 0: ++x; break;\n"
        "            case 1: --x; break;\n"
        "            case 2: ++y; break;\n"
        "            case 3: --y; break;\n"
        "        }\n"
        "        if (x % 2 == 0 && y % 2 == 0) {\n"
        "            path[pathCount][0] = x;\n"
        "            path[pathCount][1] = y;\n"
        "            ++pathCount;\n"
        "        }\n"
        "    }\n"
        "    if (pathCount > 0) {\n"
        "        int distances[100];\n"
        "        for (int i = 0; i < pathCount; ++i) {\n"
        "            int dx = path[i][0];\n"
        "            int dy = path[i][1];\n"
        "            distances[i] = dx * dx + dy * dy;\n"
        "        }\n"
        "        // Simple bubble sort to sort distances\n"
        "        for (int i = 0; i < pathCount; ++i) {\n"
        "            for (int j = i + 1; j < pathCount; ++j) {\n"
        "                if (distances[i] > distances[j]) {\n"
        "                    int temp = distances[i];\n"
        "                    distances[i] = distances[j];\n"
        "                    distances[j] = temp;\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n";

}


// Generate garbage function randomly
string generateGarbageFunction(string funcName) {
    int templateChoice = rand() % 5 + 1; // Randomly choose between templates 1 to 5
    switch (templateChoice) {
    case 1: return garbageTemplate1(funcName);
    case 2: return garbageTemplate2(funcName);
    case 3: return garbageTemplate3(funcName);
    case 4: return garbageTemplate4(funcName);
    case 5: return garbageTemplate5(funcName);
    default: return garbageTemplate1(funcName); // Fallback
    }
}


// Function to inject garbage code into the user's program
string injectGarbageCode(const std::string& code, int numFunctions) {
    std::string modifiedCode = code;
    string* funcNames = new string[numFunctions];
    for (int i = 0; i < numFunctions; i++) {
        funcNames[i] = generateRandomString(20);
    }
    //garbage fuction header 
    string garbage_header = "";
    for (int i = 0; i < numFunctions; i++) {
        garbage_header += "void "+funcNames[i] + "();\n";
    }
    std::regex includeRegex(R"(#include\s*<.*>)");
    size_t lastIncludePos = std::string::npos;
    size_t lastIncludeLength = 0;

    for (auto it = std::sregex_iterator(code.begin(), code.end(), includeRegex); it != std::sregex_iterator(); ++it) {
        lastIncludePos = it->position();
        lastIncludeLength = it->length();
    }

    if (lastIncludePos != std::string::npos) {
        modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n" + garbage_header);
    }

    // Insert garbage function definitions
    for (int i = 0; i < numFunctions; ++i) {
        modifiedCode += "\n"+generateGarbageFunction(funcNames[i]) + "\n";
    }

    // Locate the main function
    size_t mainPos = modifiedCode.find("int main");
    if (mainPos != string::npos) {
        // Locate the opening brace for the main function
        size_t bracePos = modifiedCode.find('{', mainPos);
        if (bracePos != string::npos) {
            // Verify there's no disruption to the main function's structure
            size_t closeParenPos = modifiedCode.find(')', mainPos);
            if (closeParenPos != string::npos && closeParenPos < bracePos) {
                // Construct garbage function calls
                string garbageCalls;
                for (int i = 0; i < numFunctions; ++i) {
                    garbageCalls += "    "+funcNames[i] + "();\n";
                }
                // Insert the garbage function calls into the main function
                modifiedCode.insert(bracePos + 1, "\n" + garbageCalls);
            }
        }
    }



    return modifiedCode;
}