#include <iterator_trait/iterator_trait>

#include <vector>
#include <array>
#include <cstdio>
#include <string>
#include <cctype>
#include <cstring>

int main() {
	int arr[] { 1, 2, 3, 4, 5 };

	// Construct an iterator
	int sum = 0;
	for (const auto v : iter(arr)) {
		sum += v;
	}

	std::printf("sum:  %d\n", sum);

	// Map values
	int sum2 = 0;
	for (const auto v : iter(arr).map([](int v) { return v * 2; })) {
		sum2 += v;
	}

	std::printf("sum2: %d\n", sum2);

	// Sum squares with map-fold
	int sum3 = iter(arr)
		.map([](int v) { return v * v; })
		.fold(0, [](int a, int) { return a + 1; });

	std::printf("sum3: %d\n", sum3);

	// Construct a string from an std::array + char doubling and uppercasing
	// NOTE: implicit cast from int return of std::toupper to char in fold
	auto str = iter(std::array<char, 3>{'a', 'b', 'c'})
		.map([](char v) { return std::toupper(v); })
		.fold(std::string{}, [](auto& a, char c) { a += c; return a += c; });

	std::printf("str:  %s\n", str.c_str());

	// Enumerate const char[]
	for (const auto kv : iter("ABCD").enumerate()) {
		if (kv.second)
			std::printf("%d '%c' ", (int)kv.first, kv.second);
	}

	std::putchar('\n');

	// Filter every second letter in string
	std::string str_to_filter {"Hello I'm a string"};
	auto filter_it = iter(str_to_filter).enumerate()
		.filter([](const auto& kv) { return !(kv.first & 1); })
		.map([](const auto& kv) { return kv.second; });

	std::printf("'%s' -> '", str_to_filter.data());
	for (auto c : filter_it) {
		std::putchar(c);
	}
	std::puts("'");

	// Zip two strings together
	const char* zip_string_a = "HloImasrn";
	const char* zip_string_b = "el '  tig";

	auto iter_a = iter(zip_string_a, zip_string_a + std::strlen(zip_string_a));
	auto iter_b = iter(zip_string_b, zip_string_b + std::strlen(zip_string_b));
	auto zip_string = iter_a.zip(iter_b)
		.fold(std::string{}, [](auto& s, auto ab) {
			s += ab.first;
			return s += ab.second;
		});

	std::printf("%s\n", zip_string.data());

	// Chain two strings together
	auto chain_string = iter("Hello").chain(iter(" world"))
		.filter([](char c) { return c != 0; }) // Skip the null terminator
		.fold(std::string{}, [](auto& s, auto c) { return s += c; });

	std::printf("%s\n", chain_string.data());
}


extern "C" int foo() {
	int foo[] {1, 2, 3, 4, 5};

	return iter(foo)
		.map([] (int v) { return v*v; })
		.fold(0, [] (int a, int v) { return a + v; });
}
