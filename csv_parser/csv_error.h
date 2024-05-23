/**************************************************************************
Copyright: (C) 2021 - 2023 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_ERROR_H
#define CSV_ERROR_H

#include <string>
#include <stdexcept>


/**
 * \file
 * Error handling for CSV parser.
*/


namespace Csv {



/// Exception thrown on CSV parse error.
/// Its `what()` method returns a string with a description of where the error occurred.
class ParseError : public std::runtime_error {
	public:

		/// Constructor
		ParseError(std::size_t row, std::size_t column)
				: runtime_error(createWhatString(row, column)),
				row_(row), column_(column)
		{ }


		/// Return a 0-based row number where error occurred.
		[[nodiscard]] std::size_t row() const
		{
			return row_;
		}


		/// Return a 0-based column number where error occurred.
		[[nodiscard]] std::size_t column() const
		{
			return column_;
		}


	private:

		/// Create a string for \ref what() to return.
		/// Note that here we use 1-based row/column numbers for user-friendly output.
		static std::string createWhatString(std::size_t row, std::size_t column)
		{
			return std::string("CSV parse error at row ") + std::to_string(row + 1)
					+ ", column " + std::to_string(column + 1);
		}


		std::size_t row_ = 0;  ///< 0-based row number
		std::size_t column_ = 0;  ///< 0-based column number

};




}  // end ns



#endif
