/**************************************************************************
Copyright: (C) 2021 - 2025 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_CELL_STRING_BUFFER_H
#define CSV_CELL_STRING_BUFFER_H

#include <string_view>
#include <stdexcept>
#include <array>

#include "csv_util.h"

/**
 * \file
 * Helper for compile-time buffer construction and string unescaping.
 */



namespace Csv {



/// A helper class for compile-time buffer construction and string unescaping.
/// \tparam Size Buffer size (number of characters without any terminating null).
template<std::size_t Size>
class CellStringBuffer {
	public:

		/// Constructor. Copies the cleaned-up cell data into the buffer.
		/// \param cell Cell data to clean up and copy into the buffer.
		/// \param has_escaped_quotes If true, the data in cell is cleaned up (unescaped)
		/// before being copied into the buffer.
		/// \throw std::out_of_range if the buffer is too small to hold the cleaned-up string.
		constexpr inline explicit CellStringBuffer(std::string_view cell, bool has_escaped_quotes);

		/// Return a string view to the stored data.
		[[nodiscard]] constexpr std::string_view getStringView() const;

		/// Return buffer size, as determined by the template parameter.
		[[nodiscard]] constexpr std::size_t getBufferSize() const noexcept;


	private:

		struct Buffer {
			std::array<char, Size> buffer = { };
			std::size_t size = 0;
		};

		/// Create a buffer object, with optionally cleaned-up input in it.
		/// \throw std::out_of_range if the buffer is too small to hold the cleaned-up string.
		[[nodiscard]] constexpr static Buffer prepareBuffer(std::string_view input, bool has_escaped_quotes);

		/// Unescape a string view to newly created buffer
		[[nodiscard]] constexpr static Buffer cleanString(std::string_view input);

		Buffer buffer_;
};



// ----- Implementation



template<std::size_t Size>
constexpr CellStringBuffer<Size>::CellStringBuffer(std::string_view cell, bool has_escaped_quotes)
		: buffer_(prepareBuffer(cell, has_escaped_quotes))
{ }



template<std::size_t Size>
constexpr std::string_view CellStringBuffer<Size>::getStringView() const
{
	return {buffer_.buffer.data(), buffer_.size};
}



template<std::size_t Size>
constexpr std::size_t CellStringBuffer<Size>::getBufferSize() const noexcept
{
	return Size;
}



template<std::size_t Size>
constexpr typename CellStringBuffer<Size>::Buffer CellStringBuffer<Size>::prepareBuffer(
		std::string_view input, bool has_escaped_quotes)
{
	if (has_escaped_quotes) {
		// cleanString will throw if the buffer is too small
		return cleanString(input);
	}

	if (Size < input.size()) {
		throw std::out_of_range("Insufficient buffer size");
	}
	std::array<char, Size> buffer = { };
	for (std::size_t pos = 0; pos < input.size(); ++pos) {
		buffer[pos] = input[pos];
	}
	return Buffer{buffer, input.size()};
}



template<std::size_t Size>
constexpr typename CellStringBuffer<Size>::Buffer CellStringBuffer<Size>::cleanString(std::string_view input)
{
	if (Size < getCleanStringSize(input)) {
		throw std::out_of_range("Insufficient buffer size");
	}
	std::array<char, Size> buffer = { };
	std::size_t output_pos = 0;
	for (std::size_t input_pos = 0; input_pos < input.size(); ++input_pos) {
		char c = input[input_pos];
		buffer[output_pos] = c;
		++output_pos;
		if (c == '\"' && (input_pos + 1) < input.size() && input[input_pos + 1] == '\"') {
			++input_pos;
		}
	}
	return {buffer, output_pos};
}




}  // end ns



#endif
