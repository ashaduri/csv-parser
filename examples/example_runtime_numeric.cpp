/**************************************************************************
Copyright (C) 2022 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

#include "csv_parser.h"

#include <string_view>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <fstream>



// Example of parsing using CellDoubleValue
int parseUsingCellDoubleValue(std::string_view csv_data)
{
	// Let "cell_values" be a vector of columns.
	// After parsing, each element will contain a double value.
	std::vector<std::vector<Csv::CellDoubleValue>> cell_values;

	try {
		Csv::Parser parser;

		// parseTo() throws ParseError on error.
		parser.parseTo(csv_data, cell_values);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (std::size_t column = 0; column < cell_values.size(); ++column) {
		for (std::size_t row = 0; row < cell_values[column].size(); ++row) {
			const auto& cell = cell_values[column][row];
			std::string formatted = std::to_string(cell.getValue());
			std::cout << "(row: " << (row+1) << ", col: " << (column+1) << "): " << formatted << std::endl;
		}
	}

	return EXIT_SUCCESS;
}



// Example of parsing using CellDoubleValue
int parseUsingMatrixRowMajor(std::string_view csv_data, std::size_t num_columns)
{
	// Let "cell_values" be a matrix in row-major format.
	std::vector<float> cell_values;
	Csv::MatrixInformation info;

	try {
		Csv::Parser parser;

		// parseTo() throws ParseError on error.
		info = parser.parseToMatrixRowMajor(csv_data, cell_values, std::nullopt, num_columns);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (std::size_t column = 0; column < info.getColumns(); ++column) {
		for (std::size_t row = 0; row < info.getRows(); ++row) {
			float value = cell_values.at(info.matrixIndex(row, column));
			std::string formatted = std::to_string(value);
			std::cout << "(row: " << (row+1) << ", col: " << (column+1) << "): " << formatted << std::endl;
		}
	}

	return EXIT_SUCCESS;
}



int main(int argc, char** argv)
{
	if (argc <= 2) {
		std::cout << "Usage: " << argv[0] << " <input.csv> <num_columns>" << std::endl;
		return EXIT_FAILURE;
	}

	// Load file from parameter
	std::string input_file = (argv[1] ? argv[1] : "");
	std::size_t num_columns = std::stoi(argv[2] ? argv[2] : "");

	// Read the file to string (inefficient method, but valid for this example)
	std::ifstream ifs(input_file, std::ios::binary);
	if (!ifs.is_open()) {
		std::cerr << "Failed to open input file " << input_file << std::endl;
		return EXIT_FAILURE;
	}

	std::string csv_data((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	int cell_double_value_status = parseUsingCellDoubleValue(csv_data);

	int matrix_row_major_status = parseUsingMatrixRowMajor(csv_data, num_columns);

	return cell_double_value_status || matrix_row_major_status;
}




