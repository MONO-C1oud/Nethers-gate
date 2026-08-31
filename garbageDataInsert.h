#pragma once
#include "liberaries.h"
using namespace std;

// Function to insert garbage data into a string at every `interval`
string insertGarbage(const string& input, const string& garbage, int interval) {
    string result;
    int inputLength = input.length();

    // Insert the input characters and garbage alternately
    for (int i = 0; i < inputLength; ++i) {
        result += input[i];
        // Insert garbage data at the interval
        if ((i + 1) % interval == 0) {
            result += garbage;
        }
    }
    return result;
}

// Function to remove the garbage data
string removeGarbage(const string& input, const string& garbage, int interval) {
    string result;
    int inputLength = input.length();
    int garbageLength = garbage.length();
    int skipIndex = interval + garbageLength;  // Length after which garbage is added

    // Iterate over the input string and remove the garbage data
    for (int i = 0; i < inputLength;) {
        result += input.substr(i, interval); // Add the original part of the string
        i += interval;

        // Check if garbage follows and skip it
        if (i + garbageLength <= inputLength && input.substr(i, garbageLength) == garbage) {
            i += garbageLength; // Skip the garbage
        }
    }
    return result;
}