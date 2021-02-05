/**************************************************************************
Copyright: (C) 2021 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "csv_cell.h"

#include <string>
#include <stdexcept>
#include <functional>
#include <string_view>




/// Exception thrown on CSV parse error.
class CsvParseError : public std::runtime_error {

	public:
		CsvParseError(std::size_t row, std::size_t column)
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
			return std::string("CSV parse error at row ") + std::to_string(row + 1) + ", column " + std::to_string(column + 1);
		}


		std::size_t row_ = 0;  ///< 0-based
		std::size_t column_ = 0;  ///< 0-based

};




/// The main CSV Parser class
class CsvParser {
	public:

		/// If set to true, empty cell type is a separate type from (empty) string.
		/// Default: true.
		inline constexpr void useEmptyCellType(bool use_empty_cell_type);

		/// Check whether an empty cell type is a separate type from (empty) string.
		[[nodiscard]] inline constexpr bool useEmptyCellType() const;


		/// Parse CSV string data and store the results using a callback function.
		/// Callback function signature:
		/// void func(std::size_t row, std::size_t column, std::string_view cell_data, CsvCellTypeHint hint)
		/// \throws CsvParseError
		template<typename StoreCellFunction>
		constexpr void parse(std::string_view data, StoreCellFunction storeCell) const;


		/// Parse CSV string data into a vector of columns.
		/// Accepts types like std::vector<std::vector<CsvCellReference>>.
		/// \throws CsvParseError
		template<typename Vector2D>
		void parseTo(std::string_view data, Vector2D& values) const;


	private:

		/// Parser state machine state
		enum class MachineState {
			AtCellStart,
			InLeadingWhiteSpace,
			InsideUnquotedValue,
			InsideQuotedValue,
			AfterQuotedValue,
		};


		/// Internal state maintained during parsing
		struct ParserState {
			MachineState machine_state = MachineState::AtCellStart;
			std::size_t current_row = 0, current_column = 0;
			std::string_view current_value;


			/// Switch to next column
			constexpr void switchToNextColumn()
			{
				current_value = std::string_view();
				++current_column;
			}


			/// Switch to next line.
			/// \return new current position
			[[nodiscard]] constexpr std::size_t switchToNextLine(const std::string_view& data, std::size_t current_pos)
			{
				current_value = std::string_view();
				current_column = 0;
				++current_row;
				// If it's CR, and the next character is LF, skip LF as well.
				if (auto next_char = peek(data, current_pos); data[current_pos] == '\r' && next_char == '\n') {
					++current_pos;
				}
				return current_pos;
			}


			/// Advance current value's end point by \ref by_chars
			constexpr void increaseCurrentValueSize(std::size_t by_chars)
			{
				current_value = std::string_view(current_value.data(), current_value.size() + by_chars);
			}


			/// Reset current value to specific start point and size
			constexpr void restartCurrentValue(const std::string_view& data, std::size_t current_pos, std::size_t size)
			{
				current_value = data.substr(current_pos, size);
			}

		};


		/// Read a character from \ref data at position (current_pos + advance_chars).
		/// \return std::char_traits<char>::eof() if position is past the data's contents.
		[[nodiscard]] static constexpr std::char_traits<char>::int_type peek(const std::string_view& data, std::size_t current_pos, std::size_t advance_chars = 1)
		{
			if ((current_pos + advance_chars) < data.size()) {
				return std::char_traits<char>::to_int_type(data[current_pos + advance_chars]);
			}
			return std::char_traits<char>::eof();
		}


		/// Read a character from \ref data at position \ref pos.
		/// \return std::char_traits<char>::eof() if position is past the data's contents.
		[[nodiscard]] static constexpr std::char_traits<char>::int_type readChar(const std::string_view& data, std::size_t pos)
		{
			return peek(data, pos, 0);
		}


		/// If set to true, empty cell type is a separate type from (empty) string.
		bool use_empty_cell_type_ = true;

};




// ---- Implementation



constexpr void CsvParser::useEmptyCellType(bool use_empty_cell_type)
{
	use_empty_cell_type_ = use_empty_cell_type;
}



constexpr bool CsvParser::useEmptyCellType() const
{
	return use_empty_cell_type_;
}



template <typename StoreCellFunction>
constexpr void CsvParser::parse(std::string_view data, StoreCellFunction storeCell) const
{
	ParserState state;

	for (std::size_t pos = 0; pos <= data.size(); ++pos) {
		auto current_char = readChar(data, pos);

		switch (state.machine_state) {
			// Starting the cell
			case MachineState::AtCellStart:
			{
				switch(current_char) {
					case ' ':
					case '\t':
						// Store the whitespace
						state.machine_state = MachineState::InLeadingWhiteSpace;
						state.restartCurrentValue(data, pos, 1);
						break;
					case '\"':
						// Start a quoted cell.
						// Discard the starting quote as well, but provide a "Quoted" hint when the quoted cell is stored.
						state.machine_state = MachineState::InsideQuotedValue;
						state.restartCurrentValue(data, pos + 1, 0);
						break;
					case ',':
						// Empty cell. Store the value.
						if (use_empty_cell_type_) {
							storeCell(state.current_row, state.current_column, std::string_view(), CsvCellTypeHint::Empty);
						} else {
							storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						}
						state.switchToNextColumn();
						break;
					case '\r':
					case '\n':
						// Empty cell (trailing comma; last value on the line). Store the value.
						if (use_empty_cell_type_) {
							storeCell(state.current_row, state.current_column, std::string_view(), CsvCellTypeHint::Empty);
						} else {
							storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						}
						state.machine_state = MachineState::AtCellStart;
						// Handle CRLF if needed and set the state to the next line, cell start.
						pos = state.switchToNextLine(data, pos);
						break;
					case std::char_traits<char>::eof():
						// If it's in the first column, it's a trailing newline, nothing else to do.
						// Otherwise, it's a last empty cell on the line after comma (aka trailing comma).
						if (state.current_column != 0) {
							if (use_empty_cell_type_) {
								storeCell(state.current_row, state.current_column, std::string_view(), CsvCellTypeHint::Empty);
							} else {
								storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
							}
						}
						return;
					default:
						// Start an unquoted cell
						state.machine_state = MachineState::InsideUnquotedValue;
						state.restartCurrentValue(data, pos, 1);
						break;
				}
				break;
			}

			// Only whitespace encountered in the cell so far
			case MachineState::InLeadingWhiteSpace:
			{
				switch(current_char) {
					case ' ':
					case '\t':
						// More whitespace. Append to existing leading whitespace.
						state.increaseCurrentValueSize(1);
						break;
					case '\"':
						// Quote encountered. Discard the leading whitespace, start a quoted cell.
						// Discard the starting quote as well, but provide a "Quoted" hint when the quoted cell is stored.
						state.machine_state = MachineState::InsideQuotedValue;
						state.restartCurrentValue(data, pos + 1, 0);
						break;
					case ',':
						// Whitespace-only string cell. Store the value.
						storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						state.machine_state = MachineState::AtCellStart;
						state.switchToNextColumn();
						break;
					case '\r':
					case '\n':
						// Whitespace-only string cell (last value on the line). Store the value.
						storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						state.machine_state = MachineState::AtCellStart;
						// Handle CRLF if needed and set the state to the next line, cell start.
						pos = state.switchToNextLine(data, pos);
						break;
					case std::char_traits<char>::eof():
						// Store the value, exit.
						storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						return;
					default:
						// Continue an unquoted cell
						state.machine_state = MachineState::InsideUnquotedValue;
						state.increaseCurrentValueSize(1);
						break;
				}
				break;
			}

			// We encountered non-whitespace characters in a cell and it didn't start with a quote.
			case MachineState::InsideUnquotedValue:
			{
				switch(current_char) {
					case '\"':
						// Make sure the next character is also a quote, otherwise it's a format error.
						// We don't accept unescaped double quotes in unquoted strings because it leads
						// to ambiguity.
						if (peek(data, pos) != '\"') {
							throw CsvParseError(state.current_row, state.current_column);
						}
						// Continue unquoted string, consume the second quote as well.
						++pos;
						state.increaseCurrentValueSize(2);
						break;
					case ',':
						// End of cell. Store the value.
						storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						state.machine_state = MachineState::AtCellStart;
						state.switchToNextColumn();
						break;
					case '\r':
					case '\n':
						// End of line. Store the value.
						storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						state.machine_state = MachineState::AtCellStart;
						// Handle CRLF if needed and set the state to the next line, cell start.
						pos = state.switchToNextLine(data, pos);
						break;
					case std::char_traits<char>::eof():
						// Store the value, exit.
						storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Unquoted);
						return;
					default:
						// Continue an unquoted cell
						state.increaseCurrentValueSize(1);
						break;
				}
				break;
			}

			// The cell started with an optional whitespace and a quote, we're past the first quote.
			case MachineState::InsideQuotedValue:
			{
				switch(current_char) {
					case '\"':
						// If the next character is also a quote, it's an escaped quote.
						if (peek(data, pos) == '\"') {
							// Continue quoted string, consume the second quote as well.
							++pos;
							state.increaseCurrentValueSize(2);
						} else {
							// End of quoted value. Store the value.
							// Discard the ending quote as well, but provide a "Quoted" hint when the quoted cell is stored.
							storeCell(state.current_row, state.current_column, state.current_value, CsvCellTypeHint::Quoted);
							state.machine_state = MachineState::AfterQuotedValue;
						}
						break;
					case std::char_traits<char>::eof():
						// EOF while inside a quoted cell, throw an error.
						throw CsvParseError(state.current_row, state.current_column);
					default:
						// Continue an unquoted cell
						state.increaseCurrentValueSize(1);
						break;
				}
				break;
			}

			// The quoted cell just ended
			case MachineState::AfterQuotedValue:
			{
				switch(current_char) {
					case ' ':
					case '\t':
						// Just whitespace, ignore it.
						break;
					case ',':
						// End of cell. The value has been stored already, switch to the next line.
						state.machine_state = MachineState::AtCellStart;
						state.switchToNextColumn();
						break;
					case '\r':
					case '\n':
						// End of line. The value has been stored already, switch to the next line.
						state.machine_state = MachineState::AtCellStart;
						pos = state.switchToNextLine(data, pos);
						break;
					case std::char_traits<char>::eof():
						// Nothing more to do, return.
						return;
					default:
						// Anything else is an error
						throw CsvParseError(state.current_row, state.current_column);
				}
				break;
			}
		}
	}
}



template<typename Vector2D>
void CsvParser::parseTo(std::string_view data, Vector2D& values) const
{
	Vector2D parsed_values;
	parse(data, [&](std::size_t row, std::size_t column, std::string_view cell_data, CsvCellTypeHint hint) {
		if (parsed_values.size() < (column + 1)) {
			parsed_values.resize(column + 1);
		}
		if (parsed_values[column].size() < (row + 1)) {
			parsed_values[column].resize(row + 1);
		}
		parsed_values[column][row] = {cell_data, hint};
	});
	std::swap(values, parsed_values);
}




#endif
