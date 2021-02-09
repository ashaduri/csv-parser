/**************************************************************************
Copyright (C) 2021 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

#include "csv_parser.h"

#include <string_view>
#include <cstdlib>



int main()
{
	using namespace std::string_view_literals;

	constexpr std::string_view data = "\"abc\",def\n5,6"sv;
	constexpr std::size_t columns = 2, rows = 2;

	Csv::Parser parser;

	// parse into std::array<std::array<CellStringReference, rows>, columns>
	constexpr auto matrix = parser.parseTo2DArray<columns, rows>(data);

	// Verify the data at compile time
	[[maybe_unused]] constexpr bool result = [&matrix]() constexpr {
		static_assert(matrix[0][0].getOriginalStringView() == "abc"sv);
		static_assert(matrix[1][0].getOriginalStringView() == "def"sv);
		static_assert(matrix[0][1].getOriginalStringView() == "5"sv);
		static_assert(matrix[1][1].getOriginalStringView() == "6"sv);
		return true;
	}();

	return EXIT_SUCCESS;
}



