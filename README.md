# C++17 CSV Parser (csv-parser)
Compile-time CSV parser written in C++17.

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
using namespace std::string_view_literals;

// Data to parse
std::string_view data = "abc,def\n5,6"sv;

// Let "values" be a vector of columns.
// After parsing, each element will contain a std::string_view referencing
// a part of the original data.
std::vector<std::vector<CsvCellReference>> values;

CsvParser parser;

try {
    // parseTo() throws CsvParseError on error.
    parser.parseTo(data, values);
}
catch(CsvParseError& ex) {
    std::cerr << "CSV parse error: " << ex.what() << std::endl;
    return EXIT_FAILURE;
}

assert(values.size() == 2);
assert(values[0].size() == 2);
assert(values[1].size() == 2);

assert(values[0][0].getType() == CsvCellType::String);
assert(values[1][0].getType() == CsvCellType::String);
assert(values[0][1].getType() == CsvCellType::Double);
assert(values[1][1].getType() == CsvCellType::Double);

std::cout << "Column 0, row 0: " << values[0][0].getCleanString().value() << std::endl;  // abc
std::cout << "Column 1, row 0: " << values[1][0].getCleanString().value() << std::endl;  // def
std::cout << "Column 0, row 1: " << values[0][1].getDouble().value() << std::endl;  // 5
std::cout << "Column 1, row 1: " << values[1][1].getDouble().value() << std::endl;  // 6
```

### Compile-Time Parsing

Currently, parsing at compile-time has some restrictions:
- Only string_views are supported for output (no doubles).
- Consecutive double-quotes are not collapsed in strings.

One (possibly useful) consequence of compile-time parsing is that parse error causes compilation error. 

#### Example:
``` C++
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
    
    // Condition failures cause compilation error
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
```


## Copyright

Copyright: Alexander Shaduri <ashaduri@gmail.com>   
License: Zlib
