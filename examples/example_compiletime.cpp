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

	constexpr std::string_view data =
R"(abc,5
,"with ""quote inside"
)"sv;
	constexpr std::size_t columns = 2, rows = 2;

	constexpr Csv::Parser parser;

	// parse into std::array<std::array<CellStringReference, rows>, columns>
	constexpr auto matrix = parser.parseTo2DArray<columns, rows>(data);

	// Verify the data at compile time.
	static_assert(matrix[0][0].getOriginalStringView() == "abc"sv);
	static_assert(matrix[1][0].getOriginalStringView() == "5"sv);
	static_assert(matrix[0][1].getOriginalStringView().empty());
	static_assert(matrix[1][1].getCleanStringBuffer<"with \"\"quote inside"sv.size()>().getStringView()
			== "with \"quote inside"sv);

	return EXIT_SUCCESS;
}



