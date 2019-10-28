
#include "../Epitome.h"

void check(const std::string& expected, int leftOnes, int middleZeroes) {
    Epitome toTest(leftOnes, middleZeroes);
    auto actual = toTest.toString(true);
    if(actual != expected) {
        throw std::runtime_error("Error with leftOnes=" + std::to_string(leftOnes) + " and middleZeroes=" + std::to_string(middleZeroes) + "! Expected: " + expected + ", actual: " + actual);
    }
}

int main() {
    check("11100111", 3, 2);
    check("11111111 11000000 01111111", 10, 7);
    check("11111111 11000000", 10, 6);
    check("11111111 00000000", 8, 8);
    check("11111111 00000001", 8, 7);
    check("11111111 10000001", 9, 6);
    check("11111110 00000001", 7, 8);
    check("01111111", 0, 1);
    check("00000000 00111111", 0, 10);
    check("00000000", 0, 8);
    check("00000000 00000000", 0, 16);
    check("00011111", 0, 3);
}

