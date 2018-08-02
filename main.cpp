#include <iterator_trait/iterator_trait>

#include <vector>
#include <array>
#include <cstdio>
#include <string>
#include <cctype>

int main() {
	std::vector<int> vec{ 1, 2, 3 };

	int sum = 0;
	for (const auto v : iter(vec)) {
		sum += v;
	}

	int sum2 = 0;
	for (const auto v : iter(vec).map([](int v) { return v * 2; })) {
		sum2 += v;
	}

	int sum3 = iter(vec)
		.map([](int v) { return v * 2; })
		.fold(0, [](int a, int) { return a + 1; });

	auto str = iter(std::array<char, 3>{'a', 'b', 'c'})
		.map([](char v) -> char { return std::toupper(v); })
		.fold(std::string{}, [](std::string& a, char c) { a += c; return a += c; });

	std::printf("%d\n", sum);
	std::printf("%d\n", sum2);
	std::printf("%d\n", sum3);
	std::printf("%s\n", str.c_str());

	for (const auto kv : iter("ABCD").enumerate()) {
		if (kv.second)
			std::printf("%d '%c' ", (int)kv.first, kv.second);
	}

	std::printf("\n");
	std::getchar();
}


extern "C" int foo() {
	int foo[] {1, 2, 3, 4, 5};

	return iter(foo)
		.map([] (int v) { return v*v; })
		.fold(0, [] (int a, int v) { return a + v; });
}