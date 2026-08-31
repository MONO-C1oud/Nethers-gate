// shellcodeEncoder.h
#pragma once
#include "liberaries.h"
#include <cstdlib>

using namespace std;

struct ShellcodeStub {
    string declarations;
    string decoding;
};

ShellcodeStub generateShellcodeStub(const string& binPath) {
    ShellcodeStub result;
    const string dictFile = "tmp_dict.txt";

    // Build the command string.
    string command = "python DictionShellcode.py -file \"" + binPath + "\" -lang cpp -o \"" + dictFile + "\"";
    cout << "[DEBUG] Executing command: " << command << endl;

    // Use popen to capture the output of the python command.
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        cerr << "[ERROR] Failed to run python script via popen." << endl;
        return result;
    }

    string commandOutput;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        commandOutput += buffer;
    }
    int ret = _pclose(pipe);
    if (ret != 0) {
        cerr << "[ERROR] Python script execution failed with code " << ret << endl;
        return result;
    }

    // Extract the translate_dict declaration from the captured output.
    size_t dictStart = commandOutput.find("const char* translate_dict");
    string translateDict;
    if (dictStart != string::npos) {
        size_t dictEnd = commandOutput.find("};", dictStart);
        if (dictEnd != string::npos) {
            // Include the "};" in the extracted string.
            translateDict = commandOutput.substr(dictStart, dictEnd - dictStart + 2);
        }
        else {
            cerr << "[ERROR] Could not find the end of translate_dict declaration." << endl;
            return result;
        }
    }
    else {
        cerr << "[ERROR] translate_dict declaration not found in python output." << endl;
        return result;
    }
    cout << "[DEBUG] Extracted translate_dict declaration (" << translateDict.size() << " bytes)" << endl;

    // Open and read the tmp_dict.txt file to get the word list for dict_words.
    ifstream dictStream(dictFile);
    if (!dictStream.is_open()) {
        cerr << "[ERROR] Could not open temporary file " << dictFile << endl;
        return result;
    }

    stringstream bufferStream;
    bufferStream << dictStream.rdbuf();
    string dictContent = bufferStream.str();
    dictStream.close();
    cout << "[DEBUG] Read " << dictContent.size() << " bytes from temp file" << endl;

    // Parse the wordlist into a vector of words (one per line).
    vector<string> words;
    string line;
    stringstream contentStream(dictContent);
    while (getline(contentStream, line)) {
        if (!line.empty())
            words.push_back(line);
    }
    cout << "[DEBUG] Parsed " << words.size() << " words from wordlist" << endl;

    // Build the dict_words array from the parsed words.
    string dictWordsArray = "const int SHELLCODE_LENGTH = " + to_string(words.size()) + ";\n";
    dictWordsArray += "const char* dict_words[SHELLCODE_LENGTH] = { ";
    for (size_t i = 0; i < words.size(); ++i) {
        dictWordsArray += "\"" + words[i] + "\"";
        if (i < words.size() - 1) {
            dictWordsArray += ", ";
        }
    }
    dictWordsArray += " };\n";

    // Combine the translate_dict declaration (copied as-is) and the dict_words array.
    result.declarations = translateDict + "\n" + dictWordsArray;

    // Build the decoding stub (remains unchanged).
    string decodingStub;
    decodingStub += "char shellcode[SHELLCODE_LENGTH];\n";
    decodingStub += "for (int sc_index = 0; sc_index < SHELLCODE_LENGTH; sc_index++) {\n";
    decodingStub += "    for (int dict_index = 0; dict_index < 256; dict_index++) {\n";
    decodingStub += "        if (strcmp(translate_dict[dict_index], dict_words[sc_index]) == 0) {\n";
    decodingStub += "            shellcode[sc_index] = dict_index;\n";
    decodingStub += "            break;\n";
    decodingStub += "        }\n";
    decodingStub += "    }\n";
    decodingStub += "}\n";
    result.decoding = decodingStub;

    // Cleanup: remove temporary file.
    if (remove(dictFile.c_str()) != 0) {
        cerr << "[WARN] Failed to delete temp file " << dictFile << endl;
    }

    return result;
}