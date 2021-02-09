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
#include <filesystem>
#include <fstream>



// Example of parsing predefined data and checking it at runtime
int parsePredefinedData()
{
	using namespace std::string_view_literals;

	// Data to parse
	std::string_view data = "abc,def\n5,6"sv;

	// Let "cell_refs" be a vector of columns.
	// After parsing, each element will contain a std::string_view referencing a part of the original data.
	std::vector<std::vector<Csv::CellReference>> cell_refs;

	Csv::Parser parser;

	try {
		// parseTo() throws ParseError on error.
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

	return EXIT_SUCCESS;
}



int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "File not specified, using predefined data." << std::endl;
		return parsePredefinedData();
	}

	// Load file from parameter
	std::filesystem::path input_file = (argv[1] ? argv[1] : "");
	if (!std::filesystem::exists(input_file)) {
		std::cerr << "Input file " << input_file << " does not exist." << std::endl;
		return EXIT_FAILURE;
	}

	// Read the file to string (inefficient method, but valid for this example)
	std::ifstream ifs(input_file);
	std::string csv_data((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	// Let "cell_refs" be a vector of columns.
	// After parsing, each element will contain std::string_view referencing a part of the original data.
	// Note: CellReference must NOT outlive csv_data. If it has to, use CellValue class instead.
	std::vector<std::vector<Csv::CellReference>> cell_refs;

	try {
		Csv::Parser parser;

		// parseTo() throws ParseError on error.
		parser.parseTo(csv_data, cell_refs);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (std::size_t column = 0; column < cell_refs.size(); ++column) {
		for (std::size_t row = 0; row < cell_refs[column].size(); ++row) {
			const auto& cell = cell_refs[column][row];
			std::string formatted;
			switch(cell.getType()) {
				case Csv::CellType::Empty:
					formatted = "[empty]";
					break;
				case Csv::CellType::Double:
					formatted = std::to_string(cell.getDouble().value());
					break;
				case Csv::CellType::String:
					formatted = cell.getCleanString().value();
					break;
			}
			std::cout << "(row: " << (row+1) << ", col: " << (column+1) << "): " << formatted << std::endl;
		}
	}

	return EXIT_SUCCESS;
}



