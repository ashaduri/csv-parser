/**************************************************************************
Copyright: (C) 2021 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_ERROR_H
#define CSV_ERROR_H

#include <string>
#include <stdexcept>



namespace Csv {



/// Exception thrown on CSV parse error.
class ParseError : public std::runtime_error {

	public:
		ParseError(std::size_t row, std::size_t column)
				: runtime_error(createWhatString(row, column)),
				row_(row), column_(column)
		{ }


		/// Return a 0-based row number where error occurred
		[[nodiscard]] std::size_t row() const
		{
			return row_;
		}


		/// Return a 0-based column number where error occurred
		[[nodiscard]] std::size_t column() const
		{
			return column_;
		}


	private:

		/// Create a string for \ref what() to return
		static std::string createWhatString(std::size_t row, std::size_t column)
		{
			return std::string("CSV parse error at row ") + std::to_string(row + 1)
					+ ", column " + std::to_string(column + 1);
		}


		std::size_t row_ = 0;  ///< 0-based
		std::size_t column_ = 0;  ///< 0-based

};




}  // end ns



#endif
