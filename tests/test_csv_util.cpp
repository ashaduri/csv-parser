/**************************************************************************
Copyright: (C) 2021 - 2025 Alexander Shaduri
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


	SECTION("readNumberLocale() performs as expected") {
		// Most of these tests are done in parser test above.
		REQUIRE_FALSE(Csv::readNumberLocale<double>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<double>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<double>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<double>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberLocale<double>("1"sv) == 1.);
		REQUIRE(Csv::readNumberLocale<double>("-5e+6"sv) == -5e+6);
		REQUIRE(Csv::readNumberLocale<double>("-Inf"sv) == -std::numeric_limits<double>::infinity());
		REQUIRE(std::isnan(Csv::readNumberLocale<double>("nan"sv).value()));

		REQUIRE_FALSE(Csv::readNumberLocale<int>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<int>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<int>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<int>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberLocale<int>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumberLocale<int>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<int>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<int>("nan"sv).has_value());

		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberLocale<std::int64_t>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::int64_t>("nan"sv).has_value());

		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberLocale<std::uint64_t>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberLocale<std::uint64_t>("nan"sv).has_value());
	}


	SECTION("readNumberNoLocale() performs as expected") {
		// Most of these tests are done in parser test above.
		REQUIRE_FALSE(Csv::readNumberNoLocale<double>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<double>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<double>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<double>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberNoLocale<double>("1"sv) == 1.);
		REQUIRE(Csv::readNumberNoLocale<double>("-5e+6"sv) == -5e+6);
		REQUIRE(Csv::readNumberNoLocale<double>("-Inf"sv) == -std::numeric_limits<double>::infinity());
		REQUIRE(std::isnan(Csv::readNumberNoLocale<double>("nan"sv).value()));

		REQUIRE_FALSE(Csv::readNumberNoLocale<int>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<int>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<int>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<int>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberNoLocale<int>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumberNoLocale<int>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<int>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<int>("nan"sv).has_value());

		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberNoLocale<std::int64_t>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::int64_t>("nan"sv).has_value());

		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>(""sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>("a5"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>("5a"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>("5 a"sv).has_value());
		REQUIRE(Csv::readNumberNoLocale<std::uint64_t>("1"sv) == 1);
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>("-5e+6"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>("-Inf"sv).has_value());
		REQUIRE_FALSE(Csv::readNumberNoLocale<std::uint64_t>("nan"sv).has_value());
	}
}



/// @}
