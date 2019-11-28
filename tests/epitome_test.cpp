

#include <cxxabi.h>
#include "../src/Epitome.h"

template<typename Block>
std::string expected(int leftOnes, int middleZeroes) {
	std::string s;
	for (unsigned long i = 0; i < leftOnes; i++) {
		s += "1";
	}
	for (unsigned long i = 0; i < middleZeroes; i++) {
		s += "0";
	}
	while (s.length() % (sizeof(Block) * 8) != 0) {
		s += "1";
	}
	return s;
}

template<typename Block>
void check(int leftOnes, int middleZeroes) {
	Epitome<Block> toTest(leftOnes, middleZeroes);
	auto actual = toTest.toString(false);

	auto exp = expected<Block>(leftOnes, middleZeroes);
	if (actual != exp) {
		throw std::runtime_error("Error with leftOnes=" + std::to_string(leftOnes) + " and middleZeroes=" +
								 std::to_string(middleZeroes) + "! Expected: " + exp + ", actual: " + actual);
	}
}

template<typename Block>
void checkMultiple() {
	check<Block>(3, 2);
	check<Block>(10, 7);
	check<Block>(10, 6);
	check<Block>(8, 8);
	check<Block>(8, 7);
	check<Block>(9, 6);
	check<Block>(7, 8);
	check<Block>(0, 1);
	check<Block>(0, 10);
	check<Block>(0, 8);
	check<Block>(0, 16);
	check<Block>(0, 3);
}

int main() {
	checkMultiple<std::uint8_t>();
	checkMultiple<std::uint16_t>();
	checkMultiple<std::uint32_t>();
	checkMultiple<std::uint64_t>();
}

