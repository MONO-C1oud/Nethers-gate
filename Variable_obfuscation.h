#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;
// Function to generate a random string of the specified length
string generateRandomString(int length) {
    const string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    string randomString;
    int charactersLength = characters.size();

    // Seed the random number generator
    srand(static_cast<unsigned int>(std::time(nullptr)));

    // Generate random string
    for (int i = 0; i < length; ++i) {
        randomString += characters[std::rand() % charactersLength];
    }

    return randomString;
}
