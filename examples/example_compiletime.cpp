/**************************************************************************
Copyright (C) 2021 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

#include "csv_parser.h"

#include <string_view>
#include <cstdlib>
#include <array>



int main()
{
	using namespace std::string_view_literals;

	// Assign to constexpr variable to evaluate lambda at compile time
	[[maybe_unused]] constexpr bool result = []() constexpr {
		// Data to parse
		std::string_view data = "abc,def\n5,6"sv;

		CsvParser parser;
		std::array<std::array<CsvCellStringReference, 2>, 2> matrix;

		parser.parse(data,
			[&matrix](std::size_t row, std::size_t column,
					std::string_view cell_data, [[maybe_unused]] CsvCellTypeHint hint)
					constexpr mutable
			{
				matrix[column][row] = CsvCellStringReference(cell_data);
			}
		);
		if (matrix[0][0].getOriginalStringView() != "abc"sv) {
			throw std::runtime_error("Parsing 0, 0 failed");
		}
		if (matrix[1][0].getOriginalStringView() != "def"sv) {
			throw std::runtime_error("Parsing 1, 0 failed");
		}
		if (matrix[0][1].getOriginalStringView() != "5"sv) {
			throw std::runtime_error("Parsing 0, 1 failed");
		}
		if (matrix[1][1].getOriginalStringView() != "6"sv) {
			throw std::runtime_error("Parsing 1, 1 failed");
		}
		return true;
	}();

	return EXIT_SUCCESS;
}



