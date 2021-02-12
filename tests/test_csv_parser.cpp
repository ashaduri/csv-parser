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


// Parse CSV data and extract a single value
Csv::CellReference parseSingleValue(std::string_view data)
{
	Csv::Parser parser;
	std::vector<std::vector<Csv::CellReference>> cell_refs;
	parser.parseTo(data, cell_refs);
	REQUIRE(cell_refs.size() == 1);
	REQUIRE(cell_refs.at(0).size() == 1);
	return cell_refs.at(0).at(0);
}


}




TEST_CASE("CsvParser", "[csv][parser]")
{
	SECTION("parsing empty text does nothing") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;
		REQUIRE_NOTHROW(parser.parseTo(""sv, cell_refs));
		REQUIRE(cell_refs.empty());
	}


	SECTION("can parse empty values as Empty") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;
		REQUIRE_NOTHROW(parser.parseTo(","sv, cell_refs));
		REQUIRE(cell_refs.size() == 2);
		REQUIRE(cell_refs.at(0).size() == 1);
		REQUIRE(cell_refs.at(1).size() == 1);
		REQUIRE(cell_refs.at(0).at(0).isEmpty());
		REQUIRE(cell_refs.at(1).at(0).isEmpty());
	}


	SECTION("can parse empty values as Strings") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;
		parser.useEmptyCellType(false);  // disable Empty type support
		REQUIRE_NOTHROW(parser.parseTo(","sv, cell_refs));
		REQUIRE(cell_refs.size() == 2);
		REQUIRE(cell_refs.at(0).size() == 1);
		REQUIRE(cell_refs.at(1).size() == 1);
		REQUIRE(cell_refs.at(0).at(0).getType() == Csv::CellType::String);
		REQUIRE(cell_refs.at(0).at(0).getOriginalStringView()->empty());
		REQUIRE(cell_refs.at(1).at(0).getType() == Csv::CellType::String);
		REQUIRE(cell_refs.at(1).at(0).getOriginalStringView()->empty());
	}


	SECTION("can parse Double") {
		// getDouble() returns std::nullopt on type mismatch.
		// Just in case we test the type as well.
		REQUIRE(parseSingleValue("5"sv).getType() == Csv::CellType::Double);
		REQUIRE(parseSingleValue("5"sv).getDouble() == 5.);

		REQUIRE(parseSingleValue(" 5"sv).getDouble() == 5.);  // starting with whitespace
		REQUIRE(parseSingleValue("5 "sv).getDouble() == 5.);  // ending with whitespace
		REQUIRE(parseSingleValue(" 5 "sv).getDouble() == 5.);  // ending with whitespace
		REQUIRE(parseSingleValue("\t5"sv).getDouble() == 5.);  // starting with whitespace
		REQUIRE(parseSingleValue("-4"sv).getDouble() == -4.);
		REQUIRE(parseSingleValue("3.2"sv).getDouble() == 3.2);
		REQUIRE(parseSingleValue("1."sv).getDouble() == 1.);
		REQUIRE(parseSingleValue("+0"sv).getDouble() == 0.);
		REQUIRE(parseSingleValue("-0"sv).getDouble() == 0.);
		REQUIRE(parseSingleValue("1e2"sv).getDouble() == 1e2);
		REQUIRE(parseSingleValue("3e+4"sv).getDouble() == 3e+4);
		REQUIRE(parseSingleValue("-5.6E-7"sv).getDouble() == -5.6E-7);
		REQUIRE(parseSingleValue("inf"sv).getDouble() == std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(parseSingleValue("infinity"sv).getDouble() == std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(parseSingleValue("INF"sv).getDouble() == std::numeric_limits<double>::infinity());
		REQUIRE(parseSingleValue("Inf"sv).getDouble() == std::numeric_limits<double>::infinity());  // Matlab uses this
		REQUIRE(parseSingleValue("+inf"sv).getDouble() == std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(parseSingleValue("-inf"sv).getDouble() == -std::numeric_limits<double>::infinity());  // Standard C++
		REQUIRE(parseSingleValue("-Inf"sv).getDouble() == -std::numeric_limits<double>::infinity());  // Matlab uses this
		REQUIRE(std::isnan(parseSingleValue("nan"sv).getDouble().value()) == true);  // Standard C++
		REQUIRE(std::isnan(parseSingleValue(" nan "sv).getDouble().value()) == true);  // Standard C++ with whitespace
		REQUIRE(std::isnan(parseSingleValue("NaN"sv).getDouble().value()) == true);  // Matlab uses this
		REQUIRE(std::isnan(parseSingleValue("NAN"sv).getDouble().value()) == true);
	}


	SECTION("can parse String") {
		bool has_escaped_quotes = false;

		// CellReference::getOriginalStringView() and CellReference::getCleanString() return std::nullopt on
		// type mismatch.

		// Parsing simple unquoted strings, whitespace
		REQUIRE(parseSingleValue(" "sv).getType() == Csv::CellType::String);
		REQUIRE(parseSingleValue(" "sv).getOriginalStringView() == " "sv);  // unquoted string

		REQUIRE(parseSingleValue("abc"sv).getOriginalStringView(&has_escaped_quotes) == "abc"sv);  // unquoted string
		REQUIRE_FALSE(has_escaped_quotes);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("abc def"sv).getOriginalStringView() == "abc def"sv);  // unquoted spaced string
		REQUIRE(parseSingleValue(" abc def "sv).getOriginalStringView() == " abc def "sv);  // unquoted spaced string

		// Parsing quoted string, whitespace
		REQUIRE(parseSingleValue("\"\""sv).getOriginalStringView(&has_escaped_quotes).value().empty());  // empty quotes
		REQUIRE_FALSE(has_escaped_quotes);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("\"abc\""sv).getOriginalStringView(&has_escaped_quotes) == "abc"sv);  // quoted string
		REQUIRE_FALSE(has_escaped_quotes);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("\" abc\""sv).getOriginalStringView() == " abc"sv);  // quoted string
		REQUIRE(parseSingleValue("\"abc \""sv).getOriginalStringView() == "abc "sv);  // quoted string
		REQUIRE(parseSingleValue(" \"abc\""sv).getOriginalStringView() == "abc"sv);  // quoted string preceded by whitespace
		REQUIRE(parseSingleValue("\"abc\" "sv).getOriginalStringView() == "abc"sv);  // quoted string followed by whitespace
		REQUIRE(parseSingleValue(" \"abc\" "sv).getOriginalStringView() == "abc"sv);  // quoted string surrounded by whitespace

		// Parsing unquoted strings resembling numbers
		REQUIRE(parseSingleValue("5a"sv).getOriginalStringView() == "5a"sv);  // not-quite-number unquoted string
		REQUIRE(parseSingleValue(" 5a"sv).getOriginalStringView() == " 5a"sv);  // not-quite-number unquoted string
		REQUIRE(parseSingleValue("5 a"sv).getOriginalStringView() == "5 a"sv);  // not-quite-number unquoted spaced string
		REQUIRE(parseSingleValue(" 5 a"sv).getOriginalStringView() == " 5 a"sv);  // not-quite-number unquoted spaced string
		REQUIRE(parseSingleValue("-5.6E-7 a"sv).getOriginalStringView() == "-5.6E-7 a"sv);  // not-quite-number unquoted spaced string
		REQUIRE(parseSingleValue("+ 5"sv).getOriginalStringView() == "+ 5"sv);  // not-quite-number unquoted spaced string
		REQUIRE(parseSingleValue("infi"sv).getOriginalStringView() == "infi"sv);  // not-quite-number unquoted string

		// Parsing quoted strings containing numbers
		REQUIRE(parseSingleValue("\"5\""sv).getOriginalStringView() == "5"sv);  // quoted number string
		REQUIRE(parseSingleValue(" \"5\""sv).getOriginalStringView() == "5"sv);  // quoted number string
		REQUIRE(parseSingleValue(" \" 5\""sv).getOriginalStringView() == " 5"sv);  // quoted number string
		REQUIRE(parseSingleValue("\"-5\""sv).getOriginalStringView() == "-5"sv);  // quoted number string

		// Parsing escaped quotes and unescaping them
		REQUIRE(parseSingleValue("\"\"\"\""sv).getOriginalStringView(&has_escaped_quotes) == "\"\""sv);  // quote inside quoted string
		REQUIRE(parseSingleValue("\"\"\"\""sv).getCleanString() == "\""sv);  // unescaped version
		REQUIRE(has_escaped_quotes == true);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("\"\"\"abc\"\"\""sv).getOriginalStringView(&has_escaped_quotes) == "\"\"abc\"\""sv);  // quoted string with quotes
		REQUIRE(parseSingleValue("\"\"\"abc\"\"\""sv).getCleanString() == "\"abc\""sv);  // unescaped version
		REQUIRE(has_escaped_quotes == true);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("\"abc \"\" def\""sv).getOriginalStringView(&has_escaped_quotes) == "abc \"\" def"sv);  // quote inside quoted string
		REQUIRE(parseSingleValue("\"abc \"\" def\""sv).getCleanString() == "abc \" def"sv);  // unescaped version
		REQUIRE(has_escaped_quotes == true);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("a \"\" b"sv).getOriginalStringView(&has_escaped_quotes) == "a \"\" b"sv);  // quote inside unquoted string
		REQUIRE(parseSingleValue("a \"\" b"sv).getCleanString() == "a \" b"sv);  // unescaped version
		REQUIRE(has_escaped_quotes == true);
		has_escaped_quotes = false;

		REQUIRE(parseSingleValue("a\"\""sv).getOriginalStringView(&has_escaped_quotes) == "a\"\""sv);  // escaped quote ending unquoted string
		REQUIRE(parseSingleValue("a\"\""sv).getCleanString() == "a\""sv);  // unescaped version
		REQUIRE(has_escaped_quotes == true);
		has_escaped_quotes = false;

		// Commas and newlines inside strings
		REQUIRE(parseSingleValue("\",\""sv).getOriginalStringView() == ","sv);  // comma string
		REQUIRE(parseSingleValue("\"abc,def\""sv).getOriginalStringView() == "abc,def"sv);  // comma string
		REQUIRE(parseSingleValue("\"abc\r\ndef\""sv).getOriginalStringView() == "abc\r\ndef"sv);  // multi-line string DOS is preserved
		REQUIRE(parseSingleValue("\"abc\ndef\""sv).getOriginalStringView() == "abc\ndef"sv);  // multi-line string UNIX is preserved
		REQUIRE(parseSingleValue("\"abc\rdef\""sv).getOriginalStringView() == "abc\rdef"sv);  // multi-line string MAC is preserved
		REQUIRE(parseSingleValue("\"5,\n\r6\""sv).getOriginalStringView() == "5,\n\r6"sv);  // multi-line string with comma

		REQUIRE(parseSingleValue("\"5\"\",\n\r\"\"6\""sv).getOriginalStringView() == "5\"\",\n\r\"\"6"sv);  // multi-line string with comma and escaped quotes
		REQUIRE(parseSingleValue("\"5\"\",\n\r\"\"6\""sv).getCleanString() == "5\",\n\r\"6"sv);  // multi-line string with comma
	}


	SECTION("can parse multiple values as references") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;

		SECTION("parse a single line") {
			REQUIRE_NOTHROW(parser.parseTo("a,b", cell_refs));
			REQUIRE(cell_refs.size() == 2);
			REQUIRE(cell_refs.at(0).size() == 1);
			REQUIRE(cell_refs.at(1).size() == 1);
			REQUIRE(cell_refs.at(0).at(0).getOriginalStringView() == "a"sv);
			REQUIRE(cell_refs.at(1).at(0).getOriginalStringView() == "b"sv);
		}

		SECTION("parse a single complex line") {
			REQUIRE_NOTHROW(parser.parseTo(
					"5,inf,,string,\"quoted string\",\"with\"\"quote\",\"multi\r\nline\",\"with,commas\","sv, cell_refs));

			REQUIRE(cell_refs.size() == 9);

			REQUIRE(cell_refs.at(0).size() == 1);
			REQUIRE(cell_refs.at(1).size() == 1);
			REQUIRE(cell_refs.at(2).size() == 1);
			REQUIRE(cell_refs.at(3).size() == 1);
			REQUIRE(cell_refs.at(4).size() == 1);
			REQUIRE(cell_refs.at(5).size() == 1);
			REQUIRE(cell_refs.at(6).size() == 1);
			REQUIRE(cell_refs.at(7).size() == 1);
			REQUIRE(cell_refs.at(8).size() == 1);

			REQUIRE(cell_refs.at(0).at(0).getType() == Csv::CellType::Double);
			REQUIRE(cell_refs.at(0).at(0).getDouble().value() == 5.);

			REQUIRE(cell_refs.at(1).at(0).getType() == Csv::CellType::Double);
			REQUIRE(cell_refs.at(1).at(0).getDouble().value() == std::numeric_limits<double>::infinity());

			REQUIRE(cell_refs.at(2).at(0).isEmpty());

			REQUIRE(cell_refs.at(3).at(0).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(3).at(0).getOriginalStringView() == "string"s);

			REQUIRE(cell_refs.at(4).at(0).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(4).at(0).getOriginalStringView() == "quoted string"sv);

			REQUIRE(cell_refs.at(5).at(0).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(5).at(0).getOriginalStringView() == "with\"\"quote"sv);
			REQUIRE(cell_refs.at(5).at(0).getCleanString() == "with\"quote"s);

			REQUIRE(cell_refs.at(6).at(0).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(6).at(0).getOriginalStringView() == "multi\r\nline"sv);

			REQUIRE(cell_refs.at(7).at(0).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(7).at(0).getOriginalStringView() == "with,commas"sv);

			REQUIRE(cell_refs.at(8).at(0).getType() == Csv::CellType::Empty);
		}

		SECTION("parse multiple lines") {
			REQUIRE_NOTHROW(parser.parseTo("\"multi\r\nline\"\r\ntext\nwith many\rendings\n"sv, cell_refs));

			REQUIRE(cell_refs.size() == 1);
			REQUIRE(cell_refs.at(0).size() == 4);

			REQUIRE(cell_refs.at(0).at(0).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(0).at(0).getOriginalStringView() == "multi\r\nline"sv);

			REQUIRE(cell_refs.at(0).at(1).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(0).at(1).getOriginalStringView() == "text"sv);

			REQUIRE(cell_refs.at(0).at(2).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(0).at(2).getOriginalStringView() == "with many"sv);

			REQUIRE(cell_refs.at(0).at(3).getType() == Csv::CellType::String);
			REQUIRE(cell_refs.at(0).at(3).getOriginalStringView() == "endings"sv);
		}

		SECTION("parse 2D data") {
			REQUIRE_NOTHROW(parser.parseTo("abc,def\n5,6"sv, cell_refs));

			REQUIRE(cell_refs.size() == 2);
			REQUIRE(cell_refs[0].size() == 2);
			REQUIRE(cell_refs[1].size() == 2);

			REQUIRE(cell_refs[0][0].getType() == Csv::CellType::String);
			REQUIRE(cell_refs[1][0].getType() == Csv::CellType::String);
			REQUIRE(cell_refs[0][1].getType() == Csv::CellType::Double);
			REQUIRE(cell_refs[1][1].getType() == Csv::CellType::Double);

			REQUIRE(cell_refs.at(0).at(0).getCleanString() == "abc"s);
			REQUIRE(cell_refs.at(1).at(0).getCleanString() == "def"s);
			REQUIRE(cell_refs.at(0).at(1).getDouble() == 5.);
			REQUIRE(cell_refs.at(1).at(1).getDouble() == 6.);
		}
	}


	SECTION("CellReference supports different data") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;
		REQUIRE_NOTHROW(parser.parseTo("\"a\nb\",\"c\"\"d\"\n5e6,"sv, cell_refs));

		REQUIRE(cell_refs.size() == 2);
		REQUIRE(cell_refs[0].size() == 2);
		REQUIRE(cell_refs[1].size() == 2);

		REQUIRE(cell_refs[0][0].getType() == Csv::CellType::String);
		REQUIRE(cell_refs[1][0].getType() == Csv::CellType::String);
		REQUIRE(cell_refs[0][1].getType() == Csv::CellType::Double);
		REQUIRE(cell_refs[1][1].getType() == Csv::CellType::Empty);

		{
			bool has_escaped_quotes = false;
			REQUIRE(cell_refs.at(0).at(0).getOriginalStringView(&has_escaped_quotes) == "a\nb"sv);
			REQUIRE_FALSE(has_escaped_quotes);
			REQUIRE(cell_refs.at(0).at(0).getCleanString() == "a\nb"s);
			REQUIRE_FALSE(cell_refs.at(0).at(0).isEmpty());
			REQUIRE_FALSE(cell_refs.at(0).at(0).getDouble().has_value());  // type mismatch
		}
		{
			bool has_escaped_quotes = false;
			REQUIRE(cell_refs.at(1).at(0).getOriginalStringView(&has_escaped_quotes) == "c\"\"d"sv);
			REQUIRE(has_escaped_quotes);
			REQUIRE(cell_refs.at(1).at(0).getCleanString() == "c\"d"s);
			REQUIRE_FALSE(cell_refs.at(1).at(0).isEmpty());
			REQUIRE_FALSE(cell_refs.at(1).at(0).getDouble().has_value());  // type mismatch
		}
		{
			REQUIRE_FALSE(cell_refs.at(0).at(1).getOriginalStringView().has_value());  // type mismatch
			REQUIRE_FALSE(cell_refs.at(0).at(1).getCleanString().has_value());  // type mismatch
			REQUIRE_FALSE(cell_refs.at(0).at(1).isEmpty());
			REQUIRE(cell_refs.at(0).at(1).getDouble() == 5e6);
		}
		{
			REQUIRE_FALSE(cell_refs.at(1).at(1).getOriginalStringView().has_value());  // type mismatch
			REQUIRE_FALSE(cell_refs.at(1).at(1).getCleanString().has_value());  // type mismatch
			REQUIRE(cell_refs.at(1).at(1).isEmpty());
			REQUIRE_FALSE(cell_refs.at(1).at(1).getDouble().has_value());  // type mismatch
		}
	}


	SECTION("CellValue supports different data") {
		std::vector<std::vector<Csv::CellValue>> cell_values;

		// Limit the lifetime of original data and parser, checking that values can be stored properly.
		{
			Csv::Parser parser;
			std::string data = "\"a\nb\",\"c\"\"d\"\n5e6,";
			REQUIRE_NOTHROW(parser.parseTo(data, cell_values));
		}

		REQUIRE(cell_values.size() == 2);
		REQUIRE(cell_values[0].size() == 2);
		REQUIRE(cell_values[1].size() == 2);

		REQUIRE(cell_values[0][0].getType() == Csv::CellType::String);
		REQUIRE(cell_values[1][0].getType() == Csv::CellType::String);
		REQUIRE(cell_values[0][1].getType() == Csv::CellType::Double);
		REQUIRE(cell_values[1][1].getType() == Csv::CellType::Empty);

		{
			REQUIRE(cell_values.at(0).at(0).getString() == "a\nb"s);
			REQUIRE_FALSE(cell_values.at(0).at(0).isEmpty());
			REQUIRE_FALSE(cell_values.at(0).at(0).getDouble().has_value());  // type mismatch
		}
		{
			REQUIRE(cell_values.at(1).at(0).getString() == "c\"d"s);
			REQUIRE_FALSE(cell_values.at(1).at(0).isEmpty());
			REQUIRE_FALSE(cell_values.at(1).at(0).getDouble().has_value());  // type mismatch
		}
		{
			REQUIRE_FALSE(cell_values.at(0).at(1).getString().has_value());  // type mismatch
			REQUIRE_FALSE(cell_values.at(0).at(1).isEmpty());
			REQUIRE(cell_values.at(0).at(1).getDouble() == 5e6);
		}
		{
			REQUIRE_FALSE(cell_values.at(1).at(1).getString().has_value());  // type mismatch
			REQUIRE(cell_values.at(1).at(1).isEmpty());
			REQUIRE_FALSE(cell_values.at(1).at(1).getDouble().has_value());  // type mismatch
		}
	}


	SECTION("CellDoubleValue supports different data") {
		std::vector<std::vector<Csv::CellDoubleValue>> cell_values;

		// Limit the lifetime of original data and parser, checking that values can be stored properly.
		{
			Csv::Parser parser;
			std::string data = "\"1\",\"inf\"\n5e6,";
			REQUIRE_NOTHROW(parser.parseTo(data, cell_values));
		}

		REQUIRE(cell_values.size() == 2);
		REQUIRE(cell_values[0].size() == 2);
		REQUIRE(cell_values[1].size() == 2);

		REQUIRE(cell_values.at(0).at(0).getValue() == 1);  // parses values inside quotes
		REQUIRE(cell_values.at(1).at(0).getValue() == std::numeric_limits<double>::infinity());
		REQUIRE(cell_values.at(0).at(1).getValue() == 5e6);
		REQUIRE(std::isnan(cell_values.at(1).at(1).getValue()));
	}


	SECTION("CellStringReference supports different data") {
		std::vector<std::vector<Csv::CellStringReference>> cell_refs;
		Csv::Parser parser;
		REQUIRE_NOTHROW(parser.parseTo("\"a\nb\",\"c\"\"d\"\n5e6,"sv, cell_refs));

		REQUIRE(cell_refs.size() == 2);
		REQUIRE(cell_refs[0].size() == 2);
		REQUIRE(cell_refs[1].size() == 2);

		{
			bool has_escaped_quotes = false;
			REQUIRE(cell_refs.at(0).at(0).getOriginalStringView(&has_escaped_quotes) == "a\nb"sv);
			REQUIRE_FALSE(has_escaped_quotes);
			REQUIRE(cell_refs.at(0).at(0).getCleanString() == "a\nb"s);
		}
		{
			bool has_escaped_quotes = false;
			REQUIRE(cell_refs.at(1).at(0).getOriginalStringView(&has_escaped_quotes) == "c\"\"d"sv);
			REQUIRE(has_escaped_quotes);
			REQUIRE(cell_refs.at(1).at(0).getCleanString() == "c\"d"s);

			auto large_buffer = cell_refs.at(1).at(0).getCleanStringBuffer<4>();
			REQUIRE(large_buffer.isValid());
			REQUIRE(large_buffer.getStringView() == "c\"d"sv);  // throwing
			REQUIRE(large_buffer.getOptionalStringView() == "c\"d"sv);

			auto small_buffer = cell_refs.at(1).at(0).getCleanStringBuffer<3>();
			REQUIRE_FALSE(small_buffer.isValid());
			REQUIRE_THROWS_AS(small_buffer.getStringView(), std::out_of_range);
			REQUIRE_FALSE(small_buffer.getOptionalStringView().has_value());
		}
		{
			bool has_escaped_quotes = false;
			REQUIRE(cell_refs.at(0).at(1).getOriginalStringView(&has_escaped_quotes) == "5e6"sv);
			REQUIRE_FALSE(has_escaped_quotes);
			REQUIRE(cell_refs.at(0).at(1).getCleanString() == "5e6"s);
		}
		{
			bool has_escaped_quotes = false;
			REQUIRE(cell_refs.at(1).at(1).getOriginalStringView(&has_escaped_quotes).empty());
			REQUIRE(cell_refs.at(1).at(1).getCleanString().empty());
			REQUIRE_FALSE(has_escaped_quotes);
		}
	}


	SECTION("CellStringValue supports different data") {
		std::vector<std::vector<Csv::CellStringValue>> cell_values;

		// Limit the lifetime of original data and parser, checking that values can be stored properly.
		{
			Csv::Parser parser;
			std::string data = "\"a\nb\",\"c\"\"d\"\n5e6,";
			REQUIRE_NOTHROW(parser.parseTo(data, cell_values));
		}

		REQUIRE(cell_values.size() == 2);
		REQUIRE(cell_values[0].size() == 2);
		REQUIRE(cell_values[1].size() == 2);

		REQUIRE(cell_values.at(0).at(0).getString() == "a\nb"s);
		REQUIRE(cell_values.at(1).at(0).getString() == "c\"d"s);
		REQUIRE(cell_values.at(0).at(1).getString() == "5e6"s);
		REQUIRE(cell_values.at(1).at(1).getString().empty());
	}


	SECTION("cleanString() performs as expected") {
		// Most of these tests are done in parser test above.
		REQUIRE(Csv::cleanString(""sv).empty());
		REQUIRE(Csv::cleanString("\"\""sv) == "\""s);
		REQUIRE(Csv::cleanString("a\"\"b"sv) == "a\"b"s);
		REQUIRE(Csv::cleanString("a\"\""sv) == "a\""s);
		REQUIRE(Csv::cleanString("\"\"\"\""sv) == "\"\""s);
	}


	SECTION("readDouble() performs as expected") {
		// Most of these tests are done in parser test above.
		REQUIRE_FALSE(Csv::readDouble(""sv).has_value());
		REQUIRE_FALSE(Csv::readDouble("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readDouble("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readDouble("5 a"sv).has_value());
		REQUIRE(Csv::readDouble("1"sv) == 1.);
		REQUIRE(Csv::readDouble("-5e+6"sv) == -5e+6);
		REQUIRE(Csv::readDouble("-Inf"sv) == -std::numeric_limits<double>::infinity());
		REQUIRE(std::isnan(Csv::readDouble("nan"sv).value()));
	}


	SECTION("reports parse error when invalid") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;

		SECTION("quote missing") {
			REQUIRE_THROWS_AS(parser.parseTo("\"abc"sv, cell_refs), Csv::ParseError);
		}
		SECTION("quote missing with newline") {
			REQUIRE_THROWS_AS(parser.parseTo("\"abc\n"sv, cell_refs), Csv::ParseError);
		}
		SECTION("quote missing with comma") {
			REQUIRE_THROWS_AS(parser.parseTo("\"abc,\n"sv, cell_refs), Csv::ParseError);
		}
		SECTION("quote missing on new line") {
			REQUIRE_THROWS_AS(parser.parseTo("abc,\n\"cde,"sv, cell_refs), Csv::ParseError);
		}
		SECTION("quote not escaped (middle) (in unquoted cell)") {
			// quote inside unquoted string (conflicts with "" inside unquoted string)
			REQUIRE_THROWS_AS(parser.parseTo("a\"b"sv, cell_refs), Csv::ParseError);
		}
		SECTION("escaped quote in the beginning (in unquoted cell)") {
			// escaped quote starting unquoted string (a starting quote indicates quoted string, it's an error).
			REQUIRE_THROWS_AS(parser.parseTo("\"\"a"sv, cell_refs), Csv::ParseError);
		}
		SECTION("exception object is correct") {
			try {
				parser.parseTo("ab,cd,ef\n5,6,\"7", cell_refs);
			}
			catch (Csv::ParseError& ex) {
				REQUIRE(ex.row() == 1);
				REQUIRE(ex.column() == 2);
			}
		}
	}


	SECTION("supports constexpr") {
		constexpr std::string_view data = "\"abc\",def\n\"with \"\"quote inside\",6"sv;
		constexpr Csv::Parser parser;

		// parse into std::array<std::array<CellStringReference, rows>, columns>
		constexpr auto matrix = parser.parseTo2DArray<2, 2>(data);

		static_assert(matrix[0][0].getOriginalStringView() == "abc"sv);
		static_assert(matrix[1][0].getOriginalStringView() == "def"sv);
		static_assert(matrix[0][1].getOriginalStringView() == "with \"\"quote inside"sv);
		static_assert(matrix[1][1].getOriginalStringView() == "6"sv);

		constexpr auto buffer_size = "with \"\"quote inside"sv.size();
		constexpr auto buffer = matrix[0][1].getCleanStringBuffer<buffer_size>();
		static_assert(buffer.getStringView() == "with \"quote inside"sv);
		static_assert(buffer.isValid());
		static_assert(buffer.getOptionalStringView().has_value());

		// These will fail to compile due to small buffer size
		// constexpr auto small_buffer_size = "with \"\"quote inside"sv.size() - 1;
		// constexpr auto small_buffer = matrix[0][1].getCleanStringBuffer<small_buffer_size>();
		// static_assert(small_buffer.getStringView() != "with \"quote inside"sv);
		// static_assert(!small_buffer.isValid());
		// static_assert(!small_buffer.getOptionalStringView().has_value());
	}
}



/// @}
