#pragma once
#include "liberaries.h"
#include "randomString.h"
using namespace std;
const int varNameSize = 25;

// Struct to hold both obfuscated name and value
struct ObfuscatedVariable {
    string obfuscatedType;
    string obfuscatedName;
    string obfuscatedValue;
};

// Function to extract variables and their values
unordered_map<string, ObfuscatedVariable> extractVariables(const string& code) {
    unordered_map<string, ObfuscatedVariable> variables;
    regex variablePattern(R"(\b(\w+)\s+\b(\w+)(?:\s*=\s*([^;]+))?\s*;)"); // int a = 10  ;|| int a;
    // Regex to match pointer variable declarations
    regex pointerPattern(R"(\b(\w+)\s*\*\s*\b(\w+)(?:\s*=\s*([^;]+))?\s*;)"); //int* ptr = NULL; || int *ptr;
    // Regex to match simple array declarations
    regex arrayPattern(R"(\b(\w+)\s+\b(\w+)\[(\d*)\](?:\s*=\s*([^;]+))?\s*;)");

    smatch match;
    string::const_iterator searchStart(code.cbegin());
    //string::const_iterator searchStart(code.cbegin());

    while (regex_search(searchStart, code.cend(), match, variablePattern)) {
        string varType = match[1];  // variable type (int, float, etc.)
        string varName = match[2];  // variable name
        string varValue = match[3]; // variable value
        // Obfuscate the variable name and value
        ObfuscatedVariable obfuscatedVar;
        //cout << "VarType: " << varType << endl; //const int num1 = ??; using namespace std;
        //cout << "VarName : " << varName << endl;
        //cout << "varValue : " << varValue << endl;
        if (varType == "namespace" || varType == "return" || varType == "break") {   //using namespace std;
            obfuscatedVar.obfuscatedName = varName;
        }
        else {
            obfuscatedVar.obfuscatedName = generateRandomString(varNameSize);
        }
        obfuscatedVar.obfuscatedType = varType;
        // Obfuscate the value if it exists
        if (varValue.empty()) {  //int num1;
            obfuscatedVar.obfuscatedValue = ""; // Uninitialized variable has no value
        }
        else {
            if (varType == "string") {
                varValue = varValue.substr(1, varValue.size() - 2);
                obfuscatedVar.obfuscatedValue = xor_encrypt_decrypt(varValue);
                //cout << "obfuscated values" << endl;
                //cout << varValue << endl;
                //cout << obfuscatedVar.obfuscatedValue << endl;
            }
            else
            {
                obfuscatedVar.obfuscatedValue = varValue;  //structure
            }
            
        }
        // Store variable in the map
        variables[varName] = obfuscatedVar;  //dictionary 
        // Move past the current match
        searchStart = match.suffix().first;
    }
    string::const_iterator searchStart1 = code.cbegin();

    //pointer declaration

    while (regex_search(searchStart1, code.cend(), match, pointerPattern)) {
        string varType = match[1];  // Pointer data type (e.g., int, char)
        string varName = match[2];  // Pointer variable name (e.g., ptr)
        string varValue = match[3]; // Pointer initialization value (e.g., new int[5], a string, or other data)
        // Obfuscate only if the pointer is char* or unsigned char*
        // Obfuscate the variable name and value
        //cout << "VarType: " << varType << endl; //const int num1 = ??; using namespace std;
        //cout << "VarName : " << varName << endl;
        //cout << "varValue : " << varValue << endl;
        ObfuscatedVariable obfuscatedVar;
        obfuscatedVar.obfuscatedName = generateRandomString(varNameSize);
        obfuscatedVar.obfuscatedType = varType;
        if (!varValue.empty()) {
            if (varType == "char" || varType == "unsigned char") {
                varValue = varValue.substr(1, varValue.size() - 2);
                obfuscatedVar.obfuscatedValue = xor_encrypt_decrypt(varValue);  // Obfuscate value for char* and unsigned char*
            }
            else {
                obfuscatedVar.obfuscatedValue = varValue;  // Keep original value for other pointer types
            }
        }
        else {
            obfuscatedVar.obfuscatedValue = "";
        }
        // Store the pointer variable in the map
        variables[varName] = obfuscatedVar;
        // Move past the current match
        searchStart1 = match.suffix().first;
    }

    string::const_iterator searchStart2 = code.cbegin();

    while (regex_search(searchStart2, code.cend(), match, arrayPattern)) {
        string varType = match[1];  // Data type (e.g., int, char)
        string varName = match[2];  // Array variable name (e.g., arr)
        string arraySize = match[3]; // Size of the array (e.g., 5)
        string varValue = match[4]; // Array initialization value (e.g., {1, 2, 3})

        /*cout << "Var Type  : " << varType << endl;
        cout << "Var Name : " << varName << endl;
        cout << "array size : " << arraySize << endl;
        cout << "var Value  : " << varValue << endl;*/
        // Obfuscate only if the variable is a char array or unsigned char array
        ObfuscatedVariable obfuscatedVar;
        obfuscatedVar.obfuscatedName = generateRandomString(15);
        obfuscatedVar.obfuscatedType = varType;
        if (varType == "char" || varType == "unsigned char") {
            varValue = varValue.substr(1, varValue.size() - 2);
            obfuscatedVar.obfuscatedValue = xor_encrypt_decrypt(varValue);  // Obfuscate value for char arrays
        }
        else {
            obfuscatedVar.obfuscatedValue = varValue;  // Keep original value for other array types
        }

        //designing the generic array name
        variables[varName] = obfuscatedVar;

        // Move past the current match
        searchStart2 = match.suffix().first;
    }

    return variables;
}

// Function to replace variable names in the original code
string replaceVariableNames(string code, const unordered_map<string, ObfuscatedVariable>& variables) {
    // Regex to match string literals (both single and double quotes) 
    regex stringLiteralPattern(R"((\".*?\"|\'.*?\'))");

    // Find all string literals
    smatch match;
    string processedCode;
    string::const_iterator searchStart(code.cbegin());

    // Traverse the code, excluding string literals from the replacement
    while (regex_search(searchStart, code.cend(), match, stringLiteralPattern)) {
        // Get the part before the string literal
        string beforeLiteral = match.prefix().str();

        // Replace variable names in the part before the string literal
        for (const auto& [name, obfuscatedVar] : variables) {
            regex namePattern(R"(\b)" + name + R"(\b)");
            beforeLiteral = regex_replace(beforeLiteral, namePattern, obfuscatedVar.obfuscatedName); //int num1;
        }

        // Append the modified part before the string literal and the string literal itself
        processedCode += beforeLiteral + match[0].str();  //string str = "num1+num2";

        // Move the search start past the current match
        searchStart = match.suffix().first;
    }

    // Handle the remaining part of the code after the last string literal
    string remainingCode(searchStart, code.cend());

    // Replace variable names in the remaining part
    for (const auto& [name, obfuscatedVar] : variables) {
        regex namePattern(R"(\b)" + name + R"(\b)");
        remainingCode = regex_replace(remainingCode, namePattern, obfuscatedVar.obfuscatedName);
    }

    // Append the remaining modified code
    processedCode += remainingCode;

    return processedCode;
}

// Function to replace variable values in the original code
string importValueModules(string code, const unordered_map<string, ObfuscatedVariable>& variables) {
    std::string func_code = getEncFunc();
    
    for (const auto& [name, obfuscatedVar] : variables) {
        if (obfuscatedVar.obfuscatedType == "string" || obfuscatedVar.obfuscatedType == "char" || obfuscatedVar.obfuscatedType == "unsigned char") {
            if (obfuscatedVar.obfuscatedType == "string") {
                // Create a regex to match the variable initialization (before obfuscating the name)
                regex valuePattern(R"(\b)" + name + R"(\s*=\s*([^;]+);)");   //word + name =   
                code = regex_replace(code, valuePattern, name + " = xor_encrypt_decrypt(\"" + obfuscatedVar.obfuscatedValue + "\");");
            }
            else {
                // Create a regex to match the variable initialization (before obfuscating the name)
                string result = generateRandomString(15);
                regex valuePattern(R"(\b)" + name + R"(\s*=\s*([^;]+);)");   //word + name =   
                code = regex_replace(code, valuePattern, name + " = \"" + obfuscatedVar.obfuscatedValue + "\";\n        std::string "+ result + " = xor_encrypt_decrypt(\""+obfuscatedVar.obfuscatedValue+"\");\n       "+name+" = "+result+".c_str();\n");
            }
        }
        else
        {
            continue;
        }
    }
    // Step 1: Insert `func_code` after the last `#include`.
    std::regex includeRegex(R"(#include\s*<.*>)");
    size_t lastIncludePos = std::string::npos;
    size_t lastIncludeLength = 0;

    for (auto it = std::sregex_iterator(code.begin(), code.end(), includeRegex); it != std::sregex_iterator(); ++it) {
        lastIncludePos = it->position();
        lastIncludeLength = it->length();
    }

    if (lastIncludePos != std::string::npos) {
        code.insert(lastIncludePos + lastIncludeLength, "\n" + func_code);
    }
    // Check if <string> is already included
    std::regex stringIncludeRegex(R"(#include\s*<string>)");
    if (!std::regex_search(code, stringIncludeRegex)) {
        // Add #include <string> after the last include directive or at the start
        if (lastIncludePos != std::string::npos) {
            // Insert after the last include directive
            code.insert(lastIncludePos + lastIncludeLength, "\n#include <string>");
        }
        else {
            // No #include directives found; add at the top
            code = "#include <string>\n" + code;
        }
    }
    return code;
}

//string replaceVariableValues(const string& code) {
//    // Regex for standard variable assignment (e.g., string message = "Hello";)
//    regex variablePattern(R"(\b(\w+)\s+(\w+)(?:\s*=\s*(.*?))?\s*;)");
//    // Regex for character array assignment (e.g., char arr[10] = "data";)
//    regex arrayPattern(R"(\b(\w+)\s+\b(\w+)\[(\d*)\](?:\s*=\s*([^;]+))?\s*;)");
//
//    smatch match;
//    string result;
//    string::const_iterator searchStart(code.cbegin());
//    /*
//    // First regex loop for standard variable assignments
//    while (regex_search(searchStart, code.cend(), match, variablePattern)) {
//        string type = match[1].str();
//        string name = match[2].str();
//        string value = match[3].str();
//
//        result += match.prefix().str(); // Append code before the match
//
//        // Obfuscate only string literals
//        if (!value.empty() && (type == "string" || type == "std::string")) {
//            result += name + " = xor_encrypt_decrypt(\"" + xor_encrypt_decrypt(value) + "\");\n";
//        }
//        else {
//            result += match.str(); // Keep original line
//        }
//
//        searchStart = match.suffix().first;
//    }
//
//    result += string(searchStart, code.cend()); // Append remaining code
//    */
//    // Reset iterator for array pattern matching
//    //searchStart = result.cbegin();
//    string finalResult;
//
//    // Second regex loop for character array assignments
//    while (regex_search(searchStart, code.cend(), match, arrayPattern)) {
//        string type = match[1].str();
//        string name = match[2].str();
//        string size = match[3].str();
//        string value = match[4].str();
//        cout << "Type : " << type << endl;
//        cout << "name : " << name << endl;
//        cout << "Size : " << size << endl;
//        cout << "Value : " << value << endl;
//        finalResult += match.prefix().str(); // Append code before the match
//
//        // Obfuscate only character arrays
//        if (!value.empty() && (type == "char" || type == "unsigned char")) {
//            value = value.substr(1, value.size() - 2);
//            for (int i = 0; value[i] != '\0'; i++) {
//                if (value[i] == '"' || value[i]=='\n' || value[i]=='\t' || value[i]==' ') {
//                    
//                    value.erase(i, 1);
//                }
//                //cout << value[i] << "." << endl;
//            }
//            /*for (int i = 0; value[i] != '\0'; i++) {
//                cout << value[i];
//            }*/
//            cout << endl;
//            cout << value << endl;
//            for (int j = 0; value[j] != '\0'; j++) {
//                cout << value[j]<<"." << endl;
//            }
//            string encryptedValue = xor_encrypt_decrypt(value);
//            finalResult += "std::string tempStr = xor_encrypt_decrypt(\"" + encryptedValue + "\");\n";
//            finalResult += type + " " + name + "[" + size + "] = tempStr.c_str();\n";
//        }
//        else {
//            finalResult += match.str(); // Keep original line
//        }
//
//        searchStart = match.suffix().first;
//    }
//
//    finalResult += string(searchStart, code.cend()); // Append remaining code
//
//    return finalResult;
//}