/**************************************************************************
Copyright (C) 2021 - 2025 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

#include "csv_parser.h"

#include <string_view>
#include <cstdlib>



int main()
{
	using namespace std::string_view_literals;

	constexpr std::string_view data =
R"(abc,5
,"with ""quote inside"
NaN, -Inf
)"sv;
	constexpr std::size_t columns = 2, rows = 3;

	constexpr Csv::Parser parser;

	// parse into std::array<std::array<CellStringReference, rows>, columns>
	constexpr auto matrix = parser.parseTo2DArray<rows, columns>(data);

	// Verify the data at compile time.
	static_assert(matrix[0][0].getOriginalStringView() == "abc"sv);
	static_assert(matrix[1][0].getOriginalStringView() == "5"sv);
	static_assert(matrix[0][1].getOriginalStringView().empty());
	static_assert(matrix[1][1].getCleanStringBuffer<R"(with "quote inside)"sv.size()>().getStringView()
			== R"(with "quote inside)"sv);
	static_assert(matrix[0][2].getOriginalStringView() == "NaN"sv);
	static_assert(matrix[1][2].getOriginalStringView() == " -Inf"sv);

	// To support consecutive double-quote collapsing at compile-time, allocate a compile-time
	// buffer to place the clean string inside. The buffer size has to be at least that
	// of a collapsed string value.
	// If the buffer is too small, the code will simply not compile.
	constexpr auto buffer_size = matrix[1][1].getRequiredBufferSize();  // collapsed size
	constexpr auto buffer = matrix[1][1].getCleanStringBuffer<buffer_size>();
	static_assert(buffer.getStringView() == R"(with "quote inside)"sv);

	return EXIT_SUCCESS;
}



