/**************************************************************************
Copyright: (C) 2021 - 2025 Alexander Shaduri
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




TEST_CASE("CsvParserMatrix", "[csv][parser][matrix]")
{
	SECTION("MatrixInformation::matrixIndex works correctly") {
		Csv::MatrixInformation info;

		SECTION("row major") {
			info.setOrder(Csv::MatrixOrder::RowMajor);
			info.setRows(3);
			info.setColumns(2);

			REQUIRE(info.matrixIndex(3, 1) == 7);
		}

		SECTION("column major") {
			info.setOrder(Csv::MatrixOrder::ColumnMajor);
			info.setRows(3);
			info.setColumns(2);

			REQUIRE(info.matrixIndex(3, 1) == 6);
		}

		// If rows and columns are incorrect, the behavior is undefined.
	}


	SECTION("can parse empty text into flat matrix") {
		std::vector<Csv::CellReference> matrix;
		Csv::Parser parser;
		Csv::MatrixInformation info;

		SECTION("parsing empty text does nothing (row major)") {
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(""sv, matrix));
			REQUIRE(matrix.empty());

			REQUIRE(info.getRows() == 0);
			REQUIRE(info.getColumns() == 0);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}


		SECTION("parsing empty text does nothing (row major, explicit rows and columns)") {
			const int rows = 2, columns = 3;
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(""sv, matrix, rows, columns));
			REQUIRE(matrix.empty());

			REQUIRE(info.getRows() == 0);
			REQUIRE(info.getColumns() == 0);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}


		SECTION("parsing empty text does nothing (column major)") {
			const int rows = 2;
			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(""sv, matrix, rows, std::nullopt));
			REQUIRE(matrix.empty());

			REQUIRE(info.getRows() == 0);
			REQUIRE(info.getColumns() == 0);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		}
	}


	SECTION("can parse string into flat matrix") {
		constexpr std::string_view data =
R"(abc, "def"
,"5"
"R31",6)";
		std::vector<Csv::CellReference> matrix;
		Csv::Parser parser;
		Csv::MatrixInformation info;

		SECTION("row major, automatic dimensions") {
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[2].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[3].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}

		SECTION("row major, explicit correct rows hint") {
			// Columns must be specified for rows_hint to have any real effect
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix, 3, 2));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[2].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[3].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}

		SECTION("row major, explicit incorrect (lesser) rows hint") {
			// This should still work, it's only a hint for optimization
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix, 2, 2));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[2].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[3].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}

		SECTION("row major, explicit incorrect (greater) rows hint") {
			// This should still work, it's only a hint for optimization
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix, 5, 2));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[2].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[3].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}

		SECTION("row major, explicit correct columns") {
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix, std::nullopt, 2));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[2].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[3].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}

		SECTION("row major, explicit incorrect columns, incorrect rows hint") {
			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix, 1, 3));

			REQUIRE(matrix.size() == 3 * 3);  // incorrect size
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[2].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[3].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[4].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[5].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[6].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[7].getDouble() == 6.);
			REQUIRE(matrix[8].getType() == Csv::CellType::Empty);

			REQUIRE(info.getRows() == 3);  // size / columns
			REQUIRE(info.getColumns() == 3);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		}


		SECTION("column major, correct rows") {
			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, std::nullopt));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[2].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[3].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		}

		SECTION("column major, incorrect rows") {
			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 4, std::nullopt));

			REQUIRE(matrix.size() == 4 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[2].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[3].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[4].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[5].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[6].getDouble() == 6.);
			REQUIRE(matrix[7].getType() == Csv::CellType::Empty);

			REQUIRE(info.getRows() == 4);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		}

		SECTION("column major, correct rows, correct columns hint") {
			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, 2));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[2].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[3].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		}

		SECTION("column major, correct rows, incorrect (lesser) columns hint") {
			// This must still work
			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, 1));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[2].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[3].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		}

		SECTION("column major, correct rows, incorrect (greater) columns hint") {
			// This must still work
			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, 5));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0].getOriginalStringView() == "abc"sv);
			REQUIRE(matrix[1].getType() == Csv::CellType::Empty);
			REQUIRE(matrix[2].getOriginalStringView() == "R31"sv);
			REQUIRE(matrix[3].getOriginalStringView() == "def"sv);
			REQUIRE(matrix[4].getOriginalStringView() == "5"sv);
			REQUIRE(matrix[5].getDouble() == 6.);

			REQUIRE(info.getRows() == 3);
			REQUIRE(info.getColumns() == 2);
			REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		}
	}


	SECTION("can parse to numeric flat matrix, row major") {
		constexpr std::string_view data =
R"(11, -12
21.,inf
,3.2e1)";

		Csv::Parser parser;
		Csv::MatrixInformation info;

		SECTION("matrix of floats") {
			std::vector<float> matrix;

			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0] == 11.f);
			REQUIRE(matrix[1] == -12.f);
			REQUIRE(matrix[2] == 21.f);
			REQUIRE(matrix[3] == std::numeric_limits<float>::infinity());
			REQUIRE(std::isnan(matrix[4]));
			REQUIRE(matrix[5] == 32.f);
		}

		SECTION("matrix of doubles") {
			std::vector<double> matrix;

			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0] == 11.);
			REQUIRE(matrix[1] == -12.);
			REQUIRE(matrix[2] == 21.);
			REQUIRE(matrix[3] == std::numeric_limits<double>::infinity());
			REQUIRE(std::isnan(matrix[4]));
			REQUIRE(matrix[5] == 32.);
		}

		SECTION("matrix of ints") {
			std::vector<int> matrix;

			REQUIRE_NOTHROW(info = parser.parseToVectorRowMajor(data, matrix));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0] == 11);
			REQUIRE(matrix[1] == -12);
			REQUIRE(matrix[2] == 0);  // double cannot be parsed as int
			REQUIRE(matrix[3] == 0);
			REQUIRE(matrix[4] == 0);
			REQUIRE(matrix[5] == 0);  // double cannot be parsed as int
		}

		REQUIRE(info.getOrder() == Csv::MatrixOrder::RowMajor);
		REQUIRE(info.getRows() == 3);
		REQUIRE(info.getColumns() == 2);
	}


	SECTION("can parse to numeric flat matrix, column major") {
		constexpr std::string_view data =
R"(11, -12
21.,inf
,3.2e1)";

		Csv::Parser parser;
		Csv::MatrixInformation info;

		SECTION("matrix of floats") {
			std::vector<float> matrix;

			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, std::nullopt));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0] == 11.f);
			REQUIRE(matrix[1] == 21.f);
			REQUIRE(std::isnan(matrix[2]));
			REQUIRE(matrix[3] == -12.f);
			REQUIRE(matrix[4] == std::numeric_limits<float>::infinity());
			REQUIRE(matrix[5] == 32.f);
		}

		SECTION("matrix of doubles") {
			std::vector<double> matrix;

			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, std::nullopt));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0] == 11.);
			REQUIRE(matrix[1] == 21.);
			REQUIRE(std::isnan(matrix[2]));
			REQUIRE(matrix[3] == -12.);
			REQUIRE(matrix[4] == std::numeric_limits<double>::infinity());
			REQUIRE(matrix[5] == 32.);
		}

		SECTION("matrix of ints") {
			std::vector<int> matrix;

			REQUIRE_NOTHROW(info = parser.parseToVectorColumnMajor(data, matrix, 3, std::nullopt));

			REQUIRE(matrix.size() == 3 * 2);
			REQUIRE(matrix[0] == 11);
			REQUIRE(matrix[1] == 0);
			REQUIRE(matrix[2] == 0);
			REQUIRE(matrix[3] == -12);
			REQUIRE(matrix[4] == 0);
			REQUIRE(matrix[5] == 0);
		}

		REQUIRE(info.getOrder() == Csv::MatrixOrder::ColumnMajor);
		REQUIRE(info.getRows() == 3);
		REQUIRE(info.getColumns() == 2);
	}



	SECTION("supports constexpr with 1D array (column-major)") {
		constexpr std::string_view data =
R"(abc, "def"
,"5"
"R31",6)";
		constexpr Csv::Parser parser;

		// parse into std::array<std::array<CellStringReference, rows>, columns>
		constexpr auto matrix = parser.parseToArray<3, 2>(data, Csv::MatrixOrder::ColumnMajor);

		static_assert(matrix[0].getOriginalStringView() == "abc"sv);
		static_assert(matrix[1].getOriginalStringView().empty());
		static_assert(matrix[2].getOriginalStringView() == "R31"sv);
		static_assert(matrix[3].getOriginalStringView() == "def"sv);
		static_assert(matrix[4].getOriginalStringView() == "5"sv);
		static_assert(matrix[5].getOriginalStringView() == "6"sv);

		static_assert(matrix.size() == 6);
	}


	SECTION("supports constexpr with 1D array (row-major)") {
		constexpr std::string_view data =
R"(abc, "def"
,"5"
"R31",6)";
		constexpr Csv::Parser parser;

		// parse into std::array<std::array<CellStringReference, rows>, columns>
		constexpr auto matrix = parser.parseToArray<3, 2>(data, Csv::MatrixOrder::RowMajor);

		static_assert(matrix[0].getOriginalStringView() == "abc"sv);
		static_assert(matrix[1].getOriginalStringView() == "def"sv);
		static_assert(matrix[2].getOriginalStringView().empty());
		static_assert(matrix[3].getOriginalStringView() == "5"sv);
		static_assert(matrix[4].getOriginalStringView() == "R31"sv);
		static_assert(matrix[5].getOriginalStringView() == "6"sv);

		static_assert(matrix.size() == 6);
	}



	// Compile-time parsing of integers requires C++23.
#if __cplusplus >= 202300L

	SECTION("supports constexpr integral flat matrix, row major") {
		constexpr std::string_view data =
R"(11, -12
21.,inf
,3.2e1)";

		constexpr Csv::Parser<Csv::LocaleUnawareBehaviorPolicy> parser;

		SECTION("matrix of ints") {
			constexpr auto matrix = parser.parseToArray<3, 2, int>(data, Csv::MatrixOrder::RowMajor);

			static_assert(matrix.size() == 3 * 2);
			static_assert(matrix[0] == 11);
			static_assert(matrix[1] == -12);
			static_assert(matrix[2] == 0);  // double cannot be parsed as int
			static_assert(matrix[3] == 0);
			static_assert(matrix[4] == 0);
			static_assert(matrix[5] == 0);  // double cannot be parsed as int
		}
	}


	SECTION("can parse to numeric integral flat matrix, column major") {
		constexpr std::string_view data =
R"(11, -12
21.,inf
,3.2e1)";

		constexpr Csv::Parser<Csv::LocaleUnawareBehaviorPolicy> parser;

		SECTION("matrix of ints") {
			constexpr auto matrix = parser.parseToArray<3, 2, int>(data, Csv::MatrixOrder::ColumnMajor);

			static_assert(matrix.size() == 3 * 2);
			static_assert(matrix[0] == 11);
			static_assert(matrix[1] == 0);
			static_assert(matrix[2] == 0);
			static_assert(matrix[3] == -12);
			static_assert(matrix[4] == 0);
			static_assert(matrix[5] == 0);
		}
	}
#endif

}



/// @}
