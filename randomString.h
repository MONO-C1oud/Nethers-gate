#pragma once
#include "liberaries.h"

// Function to obfuscate variable names
std::string generateRandomString(size_t length) {
    // Define the characters that can be used for the first character (alphabet only)
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Define the characters that can be used for the rest of the string (alphabet + digits)
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    // Initialize a random device and a random number generator
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> alphabetDistribution(0, alphabet.size() - 1);
    std::uniform_int_distribution<size_t> charsetDistribution(0, charset.size() - 1);

    // Generate the random string
    std::string randomString;

    // Ensure the first character is an alphabet
    randomString += alphabet[alphabetDistribution(generator)];

    // Generate the rest of the string
    for (size_t i = 1; i < length; ++i) {
        randomString += charset[charsetDistribution(generator)];
    }

    return randomString;
}

// Function to XOR a string with a key
std::string xor_encrypt_decrypt(const std::string& input) {
    const char key = 0x5A;
    std::string result = input;
    for (char& c : result) {
        c ^= key; // XOR with key
    }
    return result;
}

std::string getEncFunc() {
    std::string funcCode = "std::string xor_encrypt_decrypt(const std::string& input) {\n"
        "const char key = 0x5A;\n"
        "std::string result = input;\n"
        "for (char& c : result) {\n"
        "    c ^= key; // XOR with key\n"
        "}\n"
        "return result;\n"
        "}\n";
    return funcCode;
}