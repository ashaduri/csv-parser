# csv-parser
Compile-time CSV parser written in C++17.

### Features
- Header-only.
- Requires only standard C++ (C++17).
- Supports reading CSV data from `std::string_view` during compilation (using `constexpr`).
- Fully supports [RFC 4180](https://www.ietf.org/rfc/rfc4180.txt), including quoted values, escaped quotes, and newlines in field values.
- Liberal in terms of accepting not-quite-standard CSV files, but detects errors when needed.
- Supports Excel CSV variations.
- Supports reading data as different types (string, double, empty field) (runtime only).
- Extensively tested using [Catch2](https://github.com/catchorg/Catch2).

Copyright: Alexander Shaduri <ashaduri@gmail.com>   
License: Zlib
