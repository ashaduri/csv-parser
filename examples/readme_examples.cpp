/**************************************************************************
Copyright (C) 2022 - 2025 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

// This file contains examples which are copy-pasted into README.md

#include "csv_parser.h"

#include <vector>
#include <iostream>
#include <cassert>
#include <cstdint>


[[maybe_unused]] int readmeExampleRuntimeParsing2D()
{
	using namespace std::string_view_literals;

	// Data to parse
	std::string_view data = "abc,def\n5,6"sv;

	// Let "cell_refs" be a vector of columns.
	// After parsing, each element will contain Csv::CellReference object. If the cell data type
	// is Csv::CellType::String, Csv::CellReference object will reference a part of the original data.
	// Other Cell* types, as well as floating point and integral types can also be used here.
	std::vector<std::vector<Csv::CellReference>> cell_refs;

	Csv::Parser parser;

	try {
	    // This throws Csv::ParseError on error.
	    parser.parseTo2DVector(data, cell_refs);
	}
	catch(Csv::ParseError& ex) {
	    std::cerr << "CSV parse error: " << ex.what() << std::endl;
	    return EXIT_FAILURE;
	}

	assert(cell_refs.size() == 2);
	assert(cell_refs[0].size() == 2);
	assert(cell_refs[1].size() == 2);

	assert(cell_refs[0][0].getType() == Csv::CellType::String);
	assert(cell_refs[1][0].getType() == Csv::CellType::String);
	assert(cell_refs[0][1].getType() == Csv::CellType::Double);
	assert(cell_refs[1][1].getType() == Csv::CellType::Double);

	std::cout << "Column 0, row 0: " << cell_refs[0][0].getCleanString().value() << std::endl;  // abc
	std::cout << "Column 1, row 0: " << cell_refs[1][0].getCleanString().value() << std::endl;  // def
	std::cout << "Column 0, row 1: " << cell_refs[0][1].getDouble().value() << std::endl;  // 5
	std::cout << "Column 1, row 1: " << cell_refs[1][1].getDouble().value() << std::endl;  // 6

	return EXIT_SUCCESS;
}



[[maybe_unused]] int readmeExampleRuntimeParsing1D()
{
	using namespace std::string_view_literals;

	// Data to parse
	std::string_view data = "11,12,13\n21,22,23"sv;

	// Let matrix_data be a flat matrix of doubles in row-major order.
	// Other floating point and integral types, as well as Cell* types can also be used here.
	std::vector<double> matrix_data;

	Csv::Parser parser;
	Csv::MatrixInformation info;

	try {
	    // This throws Csv::ParseError on error.
	    info = parser.parseToVectorRowMajor(data, matrix_data);
	}
	catch(Csv::ParseError& ex) {
	    std::cerr << "CSV parse error: " << ex.what() << std::endl;
	    return EXIT_FAILURE;
	}

	assert(matrix_data.size() == 3 * 2);
	assert(info.getColumns() == 3);
	assert(info.getRows() == 2);

	std::cout << "Row 0, column 0: " << matrix_data[0] << std::endl;  // 11
	std::cout << "Row 0, column 1: " << matrix_data[1] << std::endl;  // 12
	std::cout << "Row 0, column 2: " << matrix_data[2] << std::endl;  // 13

	// matrixIndex(row, column) can be used to avoid accidental mistakes
	std::cout << "Row 1, column 0: " << matrix_data[info.matrixIndex(1, 0)] << std::endl;  // 21
	std::cout << "Row 1, column 1: " << matrix_data[info.matrixIndex(1, 1)] << std::endl;  // 22
	std::cout << "Row 1, column 2: " << matrix_data[info.matrixIndex(1, 2)] << std::endl;  // 23

	return EXIT_SUCCESS;
}



[[maybe_unused]] void readmeExampleCompiletimeParsing2D()
{
	using namespace std::string_view_literals;

	constexpr std::size_t columns = 2, rows = 2;
	constexpr std::string_view data =
	R"(abc, "def"
	"with ""quote inside",6)";

	constexpr Csv::Parser parser;

	// parse into std::array<std::array<CellStringReference, rows>, columns>
	constexpr auto matrix = parser.parseTo2DArray<rows, columns>(data);

	// Verify the data at compile time.
	// Note that consecutive double-quotes are not collapsed when using
	// getOriginalStringView(). To collapse them, use the getCleanStringBuffer()
	// approach below.
	static_assert(matrix[0][0].getOriginalStringView() == "abc"sv);
	static_assert(matrix[1][0].getOriginalStringView() == "def"sv);
	static_assert(matrix[1][1].getOriginalStringView() == "6"sv);

	// To support consecutive double-quote collapsing at compile-time, allocate a compile-time
	// buffer to place the clean string inside. The buffer size has to be at least that
	// of a collapsed string value.
	// If the buffer is too small, the code will simply not compile.
	constexpr auto buffer_size = matrix[0][1].getRequiredBufferSize();  // collapsed size
	constexpr auto buffer = matrix[0][1].getCleanStringBuffer<buffer_size>();
	static_assert(buffer.getStringView() == R"(with "quote inside)"sv);
}



[[maybe_unused]] void readmeExampleCompiletimeParsing1D()
{
	using namespace std::string_view_literals;

	constexpr std::size_t columns = 2, rows = 3;
	constexpr std::string_view data =
	R"(11, -12
21, 4
60, -10)";

	constexpr Csv::Parser<Csv::LocaleUnawareBehaviorPolicy> parser;

	// Parse into std::array<CellStringReference, rows * columns> in row-major order
	{
		constexpr auto matrix = parser.parseToArray<rows, columns>(data, Csv::MatrixOrder::RowMajor);

		static_assert(matrix[0].getOriginalStringView() == "11"sv);
		static_assert(matrix[2].getOriginalStringView() == "21"sv);
	}

	// Parse into std::array<std::int64_t, rows * columns> in column-major order.
	// Compile-time parsing to integers is supported since C++23.
#if __cplusplus >= 202300L
	{
		constexpr auto matrix = parser.parseToArray<rows, columns, std::int64_t>(data, Csv::MatrixOrder::ColumnMajor);

		static_assert(matrix[0] == 11);
		static_assert(matrix[2] == 60);
	}
#endif
}


/*
// Custom policy which uses fast_float library for parsing to floating point.
struct CustomPolicy : Csv::LocaleUnawareBehaviorPolicy {
	template<typename Number>
	static constexpr std::optional<Number> readNumber(std::string_view cell)
	{
		Number parsed_value = 0;
		auto [ptr, ec] = fast_float::from_chars(cell.begin(), cell.end(), parsed_value);
		if (ec == std::errc() && ptr == (cell.end())) {
			return parsed_value;
		}
		return std::nullopt;
	}

	template<typename CellT>
	static constexpr CellT create(std::string_view cell, Csv::CellTypeHint hint)
	{
		return readNumber<CellT>(cell).value_or(std::numeric_limits<CellT>::quiet_NaN());
	}
};


[[maybe_unused]] void readmeExampleCustomPolicy()
{
	constexpr std::string_view data =
	R"(11.6, -12.3
2e5, -inf
nan, inf)";

	// Use custom policy which uses fast_float library for parsing to floating point.
	constexpr Csv::Parser<CustomPolicy> parser;
	constexpr std::size_t columns = 2, rows = 3;

	// Parse into std::array<double, rows * columns> in row-major order
	constexpr auto matrix = parser.parseToArray<rows, columns, double>(data, Csv::MatrixOrder::RowMajor);

	static_assert(matrix[0] == 11.6);
	static_assert(matrix[3] == -std::numeric_limits<double>::infinity());
}
*/



int main()
{
	bool success = true;
	success = (readmeExampleRuntimeParsing2D() == EXIT_SUCCESS) && success;
	success = (readmeExampleRuntimeParsing1D() == EXIT_SUCCESS) && success;

	readmeExampleCompiletimeParsing2D();  // If it compiles, it's successful

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}



