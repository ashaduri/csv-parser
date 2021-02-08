/**************************************************************************
Copyright (C) 2021 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

#include "csv_parser.h"

#include <string_view>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>



int main()
{
	using namespace std::string_view_literals;

	// Data to parse
	std::string_view data = "abc,def\n5,6"sv;

	// Let "values" be a vector of columns.
	// After parsing, each element will contain a std::string_view referencing
	// a part of the original data.
	std::vector<std::vector<Csv::CellReference>> values;

	Csv::Parser parser;

	try {
		// parseTo() throws ParseError on error.
		parser.parseTo(data, values);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	assert(values.size() == 2);
	assert(values[0].size() == 2);
	assert(values[1].size() == 2);

	assert(values[0][0].getType() == Csv::CellType::String);
	assert(values[1][0].getType() == Csv::CellType::String);
	assert(values[0][1].getType() == Csv::CellType::Double);
	assert(values[1][1].getType() == Csv::CellType::Double);

	std::cout << "Column 0, row 0: " << values[0][0].getCleanString().value() << std::endl;  // abc
	std::cout << "Column 1, row 0: " << values[1][0].getCleanString().value() << std::endl;  // def
	std::cout << "Column 0, row 1: " << values[0][1].getDouble().value() << std::endl;  // 5
	std::cout << "Column 1, row 1: " << values[1][1].getDouble().value() << std::endl;  // 6

	return EXIT_SUCCESS;
}



