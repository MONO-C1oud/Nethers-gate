#include<iostream>
#include<string>
using namespace std;
class foo {
private:
    int num1;
    int num2;
public:
    foo(int input1, int input2) {
        this->num1 = input1;
        this->num2 = input2;
    }
};
//metamorphism
void func1() {
    std::cout << "Function 1 executed" << std::endl;
}

//metamorphism
void func2() {
    std::cout << "Function 2 executed" << std::endl;
}

//metamorphism
void func3() {
    std::cout << "Function 3 executed" << std::endl;
}

//metamorphism
void func5() {
    std::cout << "Function 5 executed" << std::endl;
}

// shellcode declaration

int main() { 
    // decode shellcode

    const int a=0;
    const string main_str = "hello";
    string str;
    const char *char_stearic = " the actual code";
    unsigned char buf[] =
        "\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51\x41\x50";
    cout << main_str << endl;
    cout << char_stearic << endl;

}