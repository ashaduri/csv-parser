/**************************************************************************
Copyright: (C) 2021 Alexander Shaduri
License: Zlib
***************************************************************************/

#include "csv_parser.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>
#include <vector>
#include <cmath>
#include <limits>


using namespace std::string_view_literals;
using namespace std::string_literals;


namespace {



CsvCellReference getParsedValue(std::string_view data, std::size_t row = 0, std::size_t col = 0)
{
	CsvParser parser;
	std::vector<std::vector<CsvCellReference>> values;
	parser.parseTo(data, values);
	return values.at(col).at(row);
}



double getParsedDoubleValue(std::string_view data)
{
	auto value = getParsedValue(data);
	REQUIRE(value.getType() == CsvCellType::Double);
	return value.getDouble().value();
}



std::string getParsedStringValue(std::string_view data)
{
	auto value = getParsedValue(data);
	REQUIRE(value.getType() == CsvCellType::String);
	return value.getCleanString().value();
}



void parseValues(std::string_view data, std::vector<std::vector<CsvCellReference>>& values)
{
	CsvParser parser;
	REQUIRE_NOTHROW(parser.parseTo(data, values));
}


}




TEST_CASE("CsvLoadVariants", "[csv][parser]")
{
	/*
	 * https://www.ietf.org/rfc/rfc4180.txt
	 *
	 * RFC 4180 CSV format summary:
	 * 1. Each line ends with CRLF.
	 * 2. CRLF is optional for the last line.
	 * 3. There is an optional header line (in the same format as the rest of the data).
	 * 4. Each line must have the same number of fields. Spaces are part of the cells. There is no trailing comma on the line.
	 * 5. A field _may_ be enclosed in double quotes. If the field is not enclosed in double quotes, there may not be a double quote inside.
	 * 6. Fields containing double quotes, commas, and newlines must be enclosed in double quotes.
	 * 7. Double quotes are escaped by repeating them, like "".
	 * Considerations:
	 * There may be implementations using other line breaks than CRLF.
	 *
	 * Current parser implementation details:
	 * 1. Number cells must be unquoted to be treated as numbers.
	 * 2. Number cells may be preceded and/or followed by a whitespace (for loading manually written numeric data).
	 * 3. Standard C++, Matlab, and a few implementation-specific double formats are accepted as numbers.
	 * 4. Empty unquoted cells are treated as "empty".
	 * 5. If a quoted cell is preceded and/or followed by whitespace, this whitespace is ignored.
	 * 6. A cell is treated as number if it's unquoted and looks like a string representation of a number (completely).
	 * 7. Escaped quotes inside unquoted strings are supported.
	 * 8. DOS, UNIX, MAC line endings are supported; Excel on Mac uses MAC endings.
	 * 9. Line ending format inside strings is preserved.
	 *
	 * Ambiguity notice:
	 * Empty cell (Invalid QVariant) and empty string (empty std::string) are serialized the same by Excel.
	 * We read them as Invalid QVariants, but QVariant::toByteArray() will give empty std::strings as expected.
	 */

	SECTION("can parse numbers") {
		// RFC 4180
		REQUIRE(getParsedDoubleValue("5"sv) == 5.);
		REQUIRE(getParsedDoubleValue(" 5"sv) == 5.);  // starting with whitespace
		REQUIRE(getParsedDoubleValue("5 "sv) == 5.);  // ending with whitespace
		REQUIRE(getParsedDoubleValue(" 5 "sv) == 5.);  // ending with whitespace
		REQUIRE(getParsedDoubleValue("\t5"sv) == 5.);  // starting with whitespace
		REQUIRE(getParsedDoubleValue("-4"sv) == -4.);
		REQUIRE(getParsedDoubleValue("3.2"sv) == 3.2);
		REQUIRE(getParsedDoubleValue("1."sv) == 1.);
		REQUIRE(getParsedDoubleValue("+0"sv) == 0.);
		REQUIRE(getParsedDoubleValue("-0"sv) == 0.);
		REQUIRE(getParsedDoubleValue("1e2"sv) == 1e2);
		REQUIRE(getParsedDoubleValue("3e+4"sv) == 3e+4);
		REQUIRE(getParsedDoubleValue("-5.6E-7"sv) == -5.6E-7);
		REQUIRE(getParsedDoubleValue("inf"sv) == std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(getParsedDoubleValue("infinity"sv) == std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(getParsedDoubleValue("INF"sv) == std::numeric_limits<double>::infinity());
		REQUIRE(getParsedDoubleValue("Inf"sv) == std::numeric_limits<double>::infinity());  // Matlab uses this
		REQUIRE(getParsedDoubleValue("+inf"sv) == std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(getParsedDoubleValue("-inf"sv) == -std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(getParsedDoubleValue("-Inf"sv) == -std::numeric_limits<double>::infinity());  // Matlab uses this
		REQUIRE(std::isnan(getParsedDoubleValue("nan"sv)) == true);  // Standard C++
		REQUIRE(std::isnan(getParsedDoubleValue(" nan "sv)) == true);  // Standard C++ with whitespace
		REQUIRE(std::isnan(getParsedDoubleValue("NaN"sv)) == true);  // Matlab uses this
		REQUIRE(std::isnan(getParsedDoubleValue("NAN"sv)) == true);
	}

	SECTION("can parse empty values") {
		std::vector<std::vector<CsvCellReference>> values;
		REQUIRE_NOTHROW(parseValues(","sv, values));
		REQUIRE(values.size() == 2);
		REQUIRE(values.at(0).size() == 1);
		REQUIRE(values.at(1).size() == 1);
		REQUIRE(values.at(0).at(0).isEmpty());
		REQUIRE(values.at(1).at(0).isEmpty());
	}

	SECTION("parsing empty value does nothing") {
		std::vector<std::vector<CsvCellReference>> values;
		REQUIRE_NOTHROW(parseValues(""sv, values));
		REQUIRE(values.size() == 0);
	}

	SECTION("can parse string (byte array) values") {
		REQUIRE(getParsedStringValue("\"\""sv).empty());  // empty quotes
		REQUIRE(getParsedStringValue(" "sv) == " "sv);  // unquoted string
		REQUIRE(getParsedStringValue("\"abc\""sv) == "abc"sv);  // quoted string
		REQUIRE(getParsedStringValue("\" abc\""sv) == " abc"sv);  // quoted string
		REQUIRE(getParsedStringValue("\"abc \""sv) == "abc "sv);  // quoted string
		REQUIRE(getParsedStringValue(" \"abc\""sv) == "abc"sv);  // quoted string preceded by whitespace
		REQUIRE(getParsedStringValue("\"abc\" "sv) == "abc"sv);  // quoted string followed by whitespace
		REQUIRE(getParsedStringValue(" \"abc\" "sv) == "abc"sv);  // quoted string surrounded by whitespace
		REQUIRE(getParsedStringValue("abc"sv) == "abc"sv);  // unquoted string
		REQUIRE(getParsedStringValue("abc def"sv) == "abc def"sv);  // unquoted spaced string
		REQUIRE(getParsedStringValue(" abc def "sv) == " abc def "sv);  // unquoted spaced string
		REQUIRE(getParsedStringValue("5a"sv) == "5a"sv);  // not-quite-number unquoted string
		REQUIRE(getParsedStringValue(" 5a"sv) == " 5a"sv);  // not-quite-number unquoted string
		REQUIRE(getParsedStringValue("5 a"sv) == "5 a"sv);  // not-quite-number unquoted spaced string
		REQUIRE(getParsedStringValue(" 5 a"sv) == " 5 a"sv);  // not-quite-number unquoted spaced string
		REQUIRE(getParsedStringValue("\"5\""sv) == "5"sv);  // quoted number string
		REQUIRE(getParsedStringValue(" \"5\""sv) == "5"sv);  // quoted number string
		REQUIRE(getParsedStringValue(" \" 5\""sv) == " 5"sv);  // quoted number string
		REQUIRE(getParsedStringValue("\"-5\""sv) == "-5"sv);  // quoted number string
		REQUIRE(getParsedStringValue("-5.6E-7 a"sv) == "-5.6E-7 a"sv);  // not-quite-number unquoted spaced string
		REQUIRE(getParsedStringValue("+ 5"sv) == "+ 5"sv);  // not-quite-number unquoted spaced string
		REQUIRE(getParsedStringValue("infi"sv) == "infi"sv);  // not-quite-number unquoted string
		REQUIRE(getParsedStringValue("\"\"\"\""sv) == "\""sv);  // quote inside quoted string
		REQUIRE(getParsedStringValue("\"\"\"abc\"\"\""sv) == "\"abc\""sv);  // quoted string with quotes
		REQUIRE(getParsedStringValue("\"abc \"\" def\""sv) == "abc \" def"sv);  // quote inside quoted string
		REQUIRE(getParsedStringValue("a \"\" b"sv) == "a \" b"sv);  // quote inside unquoted string
		// REQUIRE(getParsedStringValue("a\"b"sv) == "a\"b"sv);  // quote inside unquoted string (conflicts with "" inside unquoted string)
		// REQUIRE(getParsedStringValue("\"\"a"sv) == "\"a"sv);  // escaped quote starting unquoted string (a starting quote indicates quoted string, it's an error).
		REQUIRE(getParsedStringValue("a\"\""sv) == "a\""sv);  // escaped quote ending unquoted string
		REQUIRE(getParsedStringValue("\",\""sv) == ","sv);  // comma string
		REQUIRE(getParsedStringValue("\"abc,def\""sv) == "abc,def"sv);  // comma string
		REQUIRE(getParsedStringValue("\"abc\r\ndef\""sv) == "abc\r\ndef"sv);  // multi-line string DOS is preserved
		REQUIRE(getParsedStringValue("\"abc\ndef\""sv) == "abc\ndef"sv);  // multi-line string UNIX is preserved
		REQUIRE(getParsedStringValue("\"abc\rdef\""sv) == "abc\rdef"sv);  // multi-line string MAC is preserved
		REQUIRE(getParsedStringValue("\"5,\n\r6\""sv) == "5,\n\r6"sv);  // complex string
		REQUIRE(getParsedStringValue("\"5\"\",\n\r\"\"6\""sv) == "5\",\n\r\"6"sv);  // complex string
	}

	SECTION("can parse multiple values") {
		std::vector<std::vector<CsvCellReference>> values;

		SECTION("parse a single line") {
			REQUIRE_NOTHROW(parseValues("a,b", values));
			REQUIRE(values.size() == 2);
			REQUIRE(values.at(0).size() == 1);
			REQUIRE(values.at(1).size() == 1);
			REQUIRE(values.at(0).at(0).getOriginalStringView() == "a"sv);
			REQUIRE(values.at(1).at(0).getOriginalStringView() == "b"sv);
		}

		SECTION("parse a single complex line") {
			parseValues("5,inf,,string,\"quoted string\",\"with\"\"quote\",\"multi\r\nline\",\"with,commas\",", values);

			REQUIRE(values.size() == 9);

			REQUIRE(values.at(0).size() == 1);
			REQUIRE(values.at(1).size() == 1);
			REQUIRE(values.at(2).size() == 1);
			REQUIRE(values.at(3).size() == 1);
			REQUIRE(values.at(4).size() == 1);
			REQUIRE(values.at(5).size() == 1);
			REQUIRE(values.at(6).size() == 1);
			REQUIRE(values.at(7).size() == 1);
			REQUIRE(values.at(8).size() == 1);

			REQUIRE(values.at(0).at(0).getType() == CsvCellType::Double);
			REQUIRE(values.at(0).at(0).getDouble().value() == 5.);

			REQUIRE(values.at(1).at(0).getType() == CsvCellType::Double);
			REQUIRE(values.at(1).at(0).getDouble().value() == std::numeric_limits<double>::infinity());

			REQUIRE(values.at(2).at(0).isEmpty());

			REQUIRE(values.at(3).at(0).getType() == CsvCellType::String);
			REQUIRE(values.at(3).at(0).getOriginalStringView() == "string"s);

			REQUIRE(values.at(4).at(0).getType() == CsvCellType::String);
			REQUIRE(values.at(4).at(0).getOriginalStringView() == "quoted string"sv);

			REQUIRE(values.at(5).at(0).getType() == CsvCellType::String);
			REQUIRE(values.at(5).at(0).getOriginalStringView() == "with\"\"quote"sv);
			REQUIRE(values.at(5).at(0).getCleanString() == "with\"quote"s);

			REQUIRE(values.at(6).at(0).getType() == CsvCellType::String);
			REQUIRE(values.at(6).at(0).getOriginalStringView() == "multi\r\nline"sv);

			REQUIRE(values.at(7).at(0).getType() == CsvCellType::String);
			REQUIRE(values.at(7).at(0).getOriginalStringView() == "with,commas"sv);

			REQUIRE(values.at(8).at(0).getType() == CsvCellType::Empty);
		}


		SECTION("parse multiple lines") {
			parseValues("\"multi\r\nline\"\r\ntext\nwith many\rendings\n", values);

			REQUIRE(values.size() == 1);
			REQUIRE(values.at(0).size() == 4);

			REQUIRE(values.at(0).at(0).getType() == CsvCellType::String);
			REQUIRE(values.at(0).at(0).getOriginalStringView() == "multi\r\nline"sv);

			REQUIRE(values.at(0).at(1).getType() == CsvCellType::String);
			REQUIRE(values.at(0).at(1).getOriginalStringView() == "text"sv);

			REQUIRE(values.at(0).at(2).getType() == CsvCellType::String);
			REQUIRE(values.at(0).at(2).getOriginalStringView() == "with many"sv);

			REQUIRE(values.at(0).at(3).getType() == CsvCellType::String);
			REQUIRE(values.at(0).at(3).getOriginalStringView() == "endings"sv);
		}
	}


	SECTION("returns error when invalid") {
		CsvParser parser;
		std::vector<std::vector<CsvCellReference>> values;

		SECTION("quote missing") {
			REQUIRE_THROWS_AS(parser.parseTo("\"abc", values), CsvParseError);
		}
		SECTION("quote missing with newline") {
			REQUIRE_THROWS_AS(parser.parseTo("\"abc\n", values), CsvParseError);
		}
		SECTION("quote missing with comma") {
			REQUIRE_THROWS_AS(parser.parseTo("\"abc,\n", values), CsvParseError);
		}
		SECTION("quote missing on new line") {
			REQUIRE_THROWS_AS(parser.parseTo("abc,\n\"cde,", values), CsvParseError);
		}
	}


	SECTION("supports constexpr") {
		[[maybe_unused]] constexpr bool result = []() constexpr {
			CsvParser parser;
			std::array<std::array<CsvCellStringReference, 2>, 2> matrix;
			parser.parse("abc,def\n5,6",
				[&matrix](std::size_t row, std::size_t column, std::string_view cell_data, [[maybe_unused]] CsvCellTypeHint hint) constexpr mutable {
					matrix[column][row] = CsvCellStringReference(cell_data);
				}
			);
			if (matrix[0][0].getOriginalStringView() != "abc"sv) {
				throw std::runtime_error("Parsing 0, 0 failed");
			}
			if (matrix[1][0].getOriginalStringView() != "def"sv) {
				throw std::runtime_error("Parsing 0, 0 failed");
			}
			if (matrix[0][1].getOriginalStringView() != "5"sv) {
				throw std::runtime_error("Parsing 0, 0 failed");
			}
			if (matrix[1][1].getOriginalStringView() != "6"sv) {
				throw std::runtime_error("Parsing 0, 0 failed");
			}
			return true;
		}();
	}
}



/// @}
