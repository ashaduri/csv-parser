/**************************************************************************
Copyright: (C) 2021 - 2023 Alexander Shaduri
License: Zlib
***************************************************************************/

#include "csv_util.h"

#include "catch.hpp"

#include <vector>
#include <cmath>
#include <limits>


using namespace std::string_view_literals;
using namespace std::string_literals;



TEST_CASE("CsvUtilities", "[csv][util]")
{
	SECTION("cleanString() performs as expected") {
		// Most of these tests are done in parser test above.
		REQUIRE(Csv::cleanString(""sv).empty());
		REQUIRE(Csv::cleanString("\"\""sv) == "\""s);
		REQUIRE(Csv::cleanString("a\"\"b"sv) == "a\"b"s);
		REQUIRE(Csv::cleanString("a\"\""sv) == "a\""s);
		REQUIRE(Csv::cleanString("\"\"\"\""sv) == "\"\""s);
	}


	SECTION("vector2DValue() performs as expected") {
		std::vector<std::vector<double>> v = {
				{11., 21., 31.},  // column 0
				{12., 22., 32.},  // column 1
		};

		REQUIRE(Csv::vector2DValue(v, 0, 0) == 11.);
		REQUIRE(Csv::vector2DValue(v, 0, 1) == 12.);
		REQUIRE(Csv::vector2DValue(v, 1, 0) == 21.);
		REQUIRE(Csv::vector2DValue(v, 1, 1) == 22.);
		REQUIRE(Csv::vector2DValue(v, 2, 0) == 31.);
		REQUIRE(Csv::vector2DValue(v, 2, 1) == 32.);

		REQUIRE_THROWS_AS(Csv::vector2DValue(v, 4, 2), std::out_of_range);
		REQUIRE_THROWS_AS(Csv::vector2DValue(v, 1, 3), std::out_of_range);
	}


	SECTION("readNumber() performs as expected") {
		// Most of these tests are done in parser test above.
		REQUIRE_FALSE(Csv::readNumber<double>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<double>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<double>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<double>("5 a"sv).has_value());
		REQUIRE(Csv::readNumber<double>("1"sv) == 1.);
		REQUIRE(Csv::readNumber<double>("-5e+6"sv) == -5e+6);
		REQUIRE(Csv::readNumber<double>("-Inf"sv) == -std::numeric_limits<double>::infinity());
		REQUIRE(std::isnan(Csv::readNumber<double>("nan"sv).value()));

		REQUIRE_FALSE(Csv::readNumber<int>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<int>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<int>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<int>("5 a"sv).has_value());
		REQUIRE(Csv::readNumber<int>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumber<int>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<int>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumber<int>("nan"sv).has_value());
	}
}



/// @}
