/**************************************************************************
Copyright: (C) 2021 - 2023 Alexander Shaduri
License: Zlib
***************************************************************************/

#include "csv_parser.h"

#include "catch.hpp"

#include <array>
#include <vector>
#include <cmath>
#include <limits>


using namespace std::string_view_literals;
using namespace std::string_literals;



TEST_CASE("CsvCellClasses", "[csv][parser][cell]")
{
	SECTION("CellReference supports different data") {
		std::vector<std::vector<Csv::CellReference>> cell_refs;
		Csv::Parser parser;
		REQUIRE_NOTHROW(parser.parseTo2DVector("\"a\nb\",\"c\"\"d\"\n5e6,"sv, cell_refs));

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
			REQUIRE_NOTHROW(parser.parseTo2DVector(data, cell_values));
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
			REQUIRE_NOTHROW(parser.parseTo2DVector(data, cell_values));
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
		REQUIRE_NOTHROW(parser.parseTo2DVector("\"a\nb\",\"c\"\"d\"\n5e6,"sv, cell_refs));

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

			auto large_buffer = cell_refs.at(1).at(0).getCleanStringBuffer<3>();
			REQUIRE(large_buffer.getStringView() == "c\"d"sv);

			// small buffer - constructor throws.
			REQUIRE_THROWS_AS(cell_refs.at(1).at(0).getCleanStringBuffer<2>(), std::out_of_range);
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
			REQUIRE_NOTHROW(parser.parseTo2DVector(data, cell_values));
		}

		REQUIRE(cell_values.size() == 2);
		REQUIRE(cell_values[0].size() == 2);
		REQUIRE(cell_values[1].size() == 2);

		REQUIRE(cell_values.at(0).at(0).getString() == "a\nb"s);
		REQUIRE(cell_values.at(1).at(0).getString() == "c\"d"s);
		REQUIRE(cell_values.at(0).at(1).getString() == "5e6"s);
		REQUIRE(cell_values.at(1).at(1).getString().empty());
	}

}



/// @}
