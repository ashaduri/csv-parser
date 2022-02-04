# Csv::Parser (csv-parser)
***Compile-time and runtime CSV parser written in C++17***

![GitHub](https://img.shields.io/github/license/ashaduri/csv-parser)
![Language](https://img.shields.io/badge/language-ISO%20C++17-blue)


## Features
- Header-only.
- Requires only standard C++ (C++17).
- Supports reading CSV data from `std::string_view` during compilation (using `constexpr`).
- Fully supports [RFC 4180](https://www.ietf.org/rfc/rfc4180.txt), including quoted values, escaped quotes, and newlines in field values.
- Liberal in terms of accepting not-quite-standard CSV files, but detects errors when needed.
- Supports Excel CSV variations.
- Supports reading data as different types (string, double, empty field) (runtime only).
- Extensively tested using [Catch2](https://github.com/catchorg/Catch2).

## Usage Examples

### Runtime Parsing into 2D std::vector

#### Example:

``` C++
#include "csv_parser.h"

// ...

using namespace std::string_view_literals;


// Data to parse
std::string_view data = "abc,def\n5,6"sv;

// Let "cell_refs" be a vector of columns.
// After parsing, each element will contain a std::string_view referencing
// a part of the original data.
std::vector<std::vector<Csv::CellReference>> cell_refs;

Csv::Parser parser;

try {
    // parseTo() throws Csv::ParseError on error.
    parser.parseTo(data, cell_refs);
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

### Compile-Time Parsing

Currently, parsing at compile-time has some restrictions:
- Only string_views are supported for output (no doubles).
- To collapse consecutive double-quotes in strings, a compile-time-allocated buffer has to be used.

One (possibly useful) consequence of compile-time parsing is that a parse error also causes a compilation error. 

#### Example:
``` C++
#include "csv_parser.h"

// ...

using namespace std::string_view_literals;

constexpr std::string_view data =
R"(abc, "def"
"with ""quote inside",6)";

constexpr std::size_t columns = 2, rows = 2;

constexpr Csv::Parser parser;

// parse into std::array<std::array<CellStringReference, rows>, columns>
constexpr auto matrix = parser.parseTo2DArray<columns, rows>(data);

// Verify the data at compile time.
// Note that consecutive double-quotes are not collapsed when using
// getOriginalStringView(). To collapse them, use the getCleanStringBuffer()
// approach below.
static_assert(matrix[0][0].getOriginalStringView() == "abc"sv);
static_assert(matrix[1][0].getOriginalStringView() == "def"sv);
static_assert(matrix[1][1].getOriginalStringView() == "6"sv);

// To support consecutive double-quote collapsing at compile-time, allocate a compile-time
// buffer to place the clean string inside. The buffer size has to be at least that
// of an uncollapsed string value.
// If the buffer is too small, the code will simply not compile.
constexpr auto buffer_size = R"(with ""quote inside)"sv.size();  // uncollapsed size
constexpr auto buffer = matrix[0][1].getCleanStringBuffer<buffer_size>();
static_assert(buffer.getStringView() == R"(with "quote inside)"sv);
```


## Copyright

Copyright: Alexander Shaduri <ashaduri@gmail.com>   
License: Zlib
