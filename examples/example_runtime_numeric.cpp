/**************************************************************************
Copyright (C) 2022 Alexander Shaduri
License: 0BSD (Zero-Clause BSD)
***************************************************************************/

#include "csv_parser.h"

#include <string_view>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <fstream>



// Example of parsing using 2D vector of CellDoubleValue objects
int parseUsingCellDoubleValue(std::string_view csv_data)
{
	// Let "cell_values" be a vector of columns.
	// After parsing, each element will contain a double value.
	std::vector<std::vector<Csv::CellDoubleValue>> cell_values;

	try {
		Csv::Parser parser;

		// This throws ParseError on error.
		parser.parseTo2DVector(csv_data, cell_values);
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



// Example of parsing using 2D vector of double
int parseUsingDouble(std::string_view csv_data)
{
	// Let "cell_values" be a vector of columns.
	// After parsing, each element will contain a double value.
	std::vector<std::vector<double>> cell_values;

	try {
		Csv::Parser parser;

		// This throws ParseError on error.
		parser.parseTo2DVector(csv_data, cell_values);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (std::size_t column = 0; column < cell_values.size(); ++column) {
		for (std::size_t row = 0; row < cell_values[column].size(); ++row) {
			const auto& cell = cell_values[column][row];
			std::string formatted = std::to_string(cell);
			std::cout << "(row: " << (row+1) << ", col: " << (column+1) << "): " << formatted << std::endl;
		}
	}

	return EXIT_SUCCESS;
}



// Example of parsing to 1D vector (row-major ordering of flattened matrix).
// Float is used as storage type.
int parseUsingVectorRowMajor(std::string_view csv_data)
{
	// Let "cell_values" be a matrix in row-major format.
	std::vector<float> cell_values;
	Csv::MatrixInformation info;

	try {
		Csv::Parser parser;

		// This throws ParseError on error.
		info = parser.parseToVectorRowMajor(csv_data, cell_values);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (std::size_t column = 0; column < info.getColumns(); ++column) {
		for (std::size_t row = 0; row < info.getRows(); ++row) {
			auto value = cell_values.at(info.matrixIndex(row, column));
			std::string formatted = std::to_string(value);
			std::cout << "(row: " << (row+1) << ", col: " << (column+1) << "): " << formatted << std::endl;
		}
	}

	return EXIT_SUCCESS;
}



// Example of parsing to 1D vector (column-major ordering of flattened matrix).
// Long double is used as storage type.
int parseUsingVectorColumnMajor(std::string_view csv_data, std::size_t num_rows)
{
	// Let "cell_values" be a matrix in column-major format.
	std::vector<long double> cell_values;
	Csv::MatrixInformation info;

	try {
		Csv::Parser parser;

		// This throws ParseError on error.
		info = parser.parseToVectorColumnMajor(csv_data, cell_values, num_rows, std::nullopt);
	}
	catch(Csv::ParseError& ex) {
		std::cerr << "CSV parse error: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	for (std::size_t column = 0; column < info.getColumns(); ++column) {
		for (std::size_t row = 0; row < info.getRows(); ++row) {
			auto value = cell_values.at(info.matrixIndex(row, column));
			std::string formatted = std::to_string(value);
			std::cout << "(row: " << (row+1) << ", col: " << (column+1) << "): " << formatted << std::endl;
		}
	}

	return EXIT_SUCCESS;
}



int main(int argc, char** argv)
{
	if (argc <= 1) {
		std::cout << "Usage: " << argv[0] << " <input.csv> [<rows>]" << std::endl;
		return EXIT_FAILURE;
	}

	// Load file from parameter
	std::string input_file = (argv[1] ? argv[1] : "");
	int num_rows = std::stoi(argv[2] ? argv[2] : "0");

	// Read the file to string (inefficient method, but valid for this example)
	std::ifstream ifs(input_file, std::ios::binary);
	if (!ifs.is_open()) {
		std::cerr << "Failed to open input file " << input_file << std::endl;
		return EXIT_FAILURE;
	}

	std::string csv_data((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	int cell_double_value_status = parseUsingCellDoubleValue(csv_data);

	int double_status = parseUsingDouble(csv_data);

	int matrix_row_major_status = parseUsingVectorRowMajor(csv_data);

	// The number of rows *must* be given when doing column-major parsing.
	int matrix_column_major_status = EXIT_SUCCESS;
	if (num_rows > 0) {
		matrix_column_major_status = parseUsingVectorColumnMajor(csv_data, num_rows);
	}

	return cell_double_value_status || double_status || matrix_row_major_status || matrix_column_major_status;
}




