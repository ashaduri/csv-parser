# Csv::Parser (csv-parser)
***Compile-time and runtime CSV parser written in Modern C++***

[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/ashaduri/csv-parser?label=Version)](https://github.com/ashaduri/csv-parser)
![GitHub](https://img.shields.io/github/license/ashaduri/csv-parser)
![Language](https://img.shields.io/badge/language-Modern%20C++-blue)


## Features
- Header-only.
- Requires only standard C++ (minimum C++17, with C++23 support providing additional functionality).
- Supports reading CSV data from `std::string_view` at compile-time.
- Fully supports [RFC 4180](https://www.ietf.org/rfc/rfc4180.txt), including quoted values, escaped quotes, and newlines in field values.
- Liberal in terms of accepting not-quite-standard CSV files, but detects errors when needed.
- Supports Excel CSV variations.
- Supports reading data as different types (string, double, empty field), with some restrictions when parsing at 
  compile-time.
- Modular design allows for easy extension of the library.
- Extensively tested using [Catch2](https://github.com/catchorg/Catch2).

## API Reference

Automatically generated [API reference](https://ashaduri.github.io/csv-parser/) describes the public API in detail.

## Usage Examples

### Runtime Parsing into 2D std::vector

#### Example:

```cpp
#include "csv_parser.h"

// ...

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
```

### Runtime Parsing of Numeric Matrix Into 1D Vector With Row-Major Order 

#### Example:

```cpp
#include "csv_parser.h"

// ...

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
```

### Compile-Time Parsing Into 2D std::array

Currently, parsing at compile-time has some restrictions:
- To collapse consecutive double-quotes in strings, a compile-time-allocated buffer has to be used.
- By default, only string_views are supported for output (no numeric types).
  - If using C++23 or later, integral types are supported as well.
  - If compile-time parsing of floating point numbers is needed,
    [fast_float](https://github.com/fastfloat/fast_float) can be plugged in.

One (possibly useful) consequence of compile-time parsing is that a parse error also causes a compilation error. 

#### Example:
```cpp
#include "csv_parser.h"

// ...

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
```

### Compile-Time Parsing of Integral Matrix Into 1D Vector

The library can be used to parse CSV data at compile-time into a 1D vector, in row-major
or column-major order.
Additionally, compile-time parsing of integral types is supported since C++23.

#### Example:
```cpp
#include "csv_parser.h"

// ...

using namespace std::string_view_literals;

constexpr std::size_t columns = 2, rows = 3;
constexpr std::string_view data =
R"(11, -12
21, 4
60, -10)";

// Use Csv::LocaleUnawareBehaviorPolicy to avoid locale issues and allow compile-time integer parsing.
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
```


### Compile-Time Parsing of Numeric Matrix Using External Library for Floating Point Numbers

A library like [fast_float](https://github.com/fastfloat/fast_float) can be used to parse floating point 
numbers at compile-time. This is done by creating a custom policy that implements the `readNumber` and
`create` methods.
This approach also allows for improving the performance of parsing floating point numbers compared
to the standard library.

#### Example:
```cpp
#include "csv_parser.h"
#include "fast_float/fast_float.h"

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


void parse()
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
```


## Copyright

Copyright: Alexander Shaduri <ashaduri@gmail.com>   
License: Zlib
