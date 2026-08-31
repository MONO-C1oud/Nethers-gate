#pragma once
#include "liberaries.h"
#include "randomString.h"
#include <TlHelp32.h>
using namespace std;

// Set to store all Windows API functions
std::set<std::string> windowsApiSet;

std::string getImportantModule() {
    std::string funcCode = R"(
HMODULE GetModuleByExport(const std::string& apiName) { 
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            HMODULE hModule = GetModuleHandle(moduleEntry.szModule);
            if (hModule) {
                if (GetProcAddress(hModule, apiName.c_str())) {
                    CloseHandle(hSnapshot);
                    return hModule;
                }
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
    }

    CloseHandle(hSnapshot);
    return nullptr;
}

void* ResolveAPI(const std::string& apiName) {
    HMODULE hModule = GetModuleByExport(apiName);
    if (!hModule) {
        std::cerr << "Failed to find module for API: " << apiName << std::endl;
        return nullptr;
    }

    return (void*)GetProcAddress(hModule, apiName.c_str());
}

#define CALL_API(api_name, ...) ((decltype(&api_name))ResolveAPI(#api_name))(__VA_ARGS__)
    )";

    return funcCode;
}


// Function to load exported functions from a DLL
void LoadExportedFunctions(const std::string& dllName) {
    HMODULE hModule = LoadLibraryA(dllName.c_str());
    if (!hModule) return;

    auto pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
    auto pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDOSHeader->e_lfanew);
    auto pExportDir = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)hModule + pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    auto pNames = (DWORD*)((BYTE*)hModule + pExportDir->AddressOfNames);

    for (DWORD i = 0; i < pExportDir->NumberOfNames; i++) {
        const char* functionName = (char*)hModule + pNames[i];
        windowsApiSet.insert(functionName);
    }

    FreeLibrary(hModule);
}


void LoadWindowsAPIs() {
    LoadExportedFunctions("kernel32.dll");
    LoadExportedFunctions("user32.dll");
    LoadExportedFunctions("ntdll.dll");
    LoadExportedFunctions("advapi32.dll");
}

bool IsWindowsAPI(const std::string& funcName) {
    return windowsApiSet.find(funcName) != windowsApiSet.end();
}

string replaceFunctionCalls(const string& code) {

    LoadWindowsAPIs();
    // Enhanced regex to handle both assignments and non-assignment function calls
    regex pattern(R"(\b(\w+)?\s*(?:=\s*)?\b(\w+)\s*\((.*)\)\s*;)");

    smatch match;
    string result;
    string::const_iterator searchStart(code.cbegin());

    while (regex_search(searchStart, code.cend(), match, pattern)) {
        string resultVar = match[1].str();
        string apiName = match[2].str();
        //string funcName = match[3].str();
        string parameters = match[3].str();

        //cout << "resultVar: " << (resultVar.empty() ? "N/A" : resultVar) << endl;
        //cout << "apiName: " << (apiName.empty() ? "N/A" : apiName) << endl;
        //cout << "Args: " << parameters << endl;
        
        if (IsWindowsAPI(apiName)) {
            result += match.prefix().str(); // Append code before the match

            if (!resultVar.empty()) {
                result += resultVar + " = CALL_API(" + apiName + ", " + parameters + ");\n";
            }
            else {
                result += "CALL_API(" + apiName + ", " + parameters + ");\n";
            }
        }
        else {
            result += match.prefix().str() + match.str(); // Keep original code
        }
        searchStart = match.suffix().first;
    }

    result += string(searchStart, code.cend());
    //cout << result;
    return result;
}
string placeFunctionData(const string& code) {
    
    std::string func_code = getImportantModule();
    // Step 1: Insert code after the last `#include`.
    std::regex includeRegex(R"(#include\s*<.*>)");
    std::string modifiedCode = code;

    //modifiedCode = replaceFunctionCalls(modifiedCode);

    size_t lastIncludePos = std::string::npos;
    size_t lastIncludeLength = 0;

    for (auto it = std::sregex_iterator(modifiedCode.begin(), modifiedCode.end(), includeRegex); it != std::sregex_iterator(); ++it) {
        lastIncludePos = it->position();
        lastIncludeLength = it->length();
    }

    if (lastIncludePos != std::string::npos) {
        modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n" + func_code);
    }

    // Check if <vector> is already included
    std::regex vectorIncludeRegex(R"(#include\s*<\s*TlHelp32.h\s*>)");
    if (!std::regex_search(modifiedCode, vectorIncludeRegex)) {
        // Add #include <vector> after the last include directive or at the start
        if (lastIncludePos != std::string::npos) {
            // Insert after the last include directive
            modifiedCode.insert(lastIncludePos + lastIncludeLength, "\n#include <TlHelp32.h>");
        }
        else {
            // No #include directives found; add at the top
            modifiedCode = "#include <TlHelp32.h>\n" + modifiedCode;
        }
    }
    

    return modifiedCode;
}


/*
// Resolve API dynamically
void* ResolveAPI(const std::string& apiName) {
    for (const auto& dll : { "kernel32.dll", "user32.dll", "ntdll.dll", "advapi32.dll" }) {
        HMODULE hModule = LoadLibraryA(dll);
        if (hModule) {
            void* pFunc = GetProcAddress(hModule, apiName.c_str());
            if (pFunc) return pFunc;
        }
    }
    return nullptr;
}

// Function to get handle of a module by its export name
HMODULE GetModuleByExport(const std::string& apiName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(MODULEENTRY32);

    if (Module32First(hSnapshot, &moduleEntry)) {
        do {
            HMODULE hModule = GetModuleHandle(moduleEntry.szModule);
            if (hModule) {
                // Check if the API is present in the export table of the current module
                if (GetProcAddress(hModule, apiName.c_str())) {
                    CloseHandle(hSnapshot);
                    return hModule;
                }
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
    }

    CloseHandle(hSnapshot);
    return nullptr;
}

// General resolver for any API without manual mapping
void* ResolveAPI(const std::string& apiName) {
    HMODULE hModule = GetModuleByExport(apiName);
    if (!hModule) {
        std::cerr << "Failed to find module for API: " << apiName << std::endl;
        return nullptr;
    }

    return (void*)GetProcAddress(hModule, apiName.c_str());
}





// Macro to call API obfuscated

#define CALL_API(api_name, ...) ((decltype(&api_name))ResolveAPI(#api_name))(__VA_ARGS__)

int main() {
    LoadWindowsAPIs();  // Load all Windows API functions into the set

    std::string function1 = "OpenProcess";
    std::string function2 = "MyCustomFunction";

    if (IsWindowsAPI(function1)) {
        std::cout << function1 << " is a Windows API." << std::endl;
    }

    if (!IsWindowsAPI(function2)) {
        std::cout << function2 << " is a custom function." << std::endl;
    }

    // Call the API obfuscated
    DWORD pid = 1234;
    HANDLE hProcess = CALL_API(OpenProcess, PROCESS_ALL_ACCESS, FALSE, pid);
    CALL_API(CloseHandle, hProcess);

    return 0;
}



*/