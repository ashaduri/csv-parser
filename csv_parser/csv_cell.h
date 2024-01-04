/**************************************************************************
Copyright: (C) 2021 - 2023 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_CELL_H
#define CSV_CELL_H

#include <string_view>
#include <string>
#include <optional>
#include <algorithm>
#include <variant>
#include <limits>
#include <stdexcept>
#include <array>

#include "csv_util.h"


namespace Csv {


/// Type hint associated with the cell to determine the type of the cell value
enum class CellTypeHint {
	Empty,  ///< Empty data (no quotes, no whitespace)
	StringWithEscapedQuotes,  ///< Quoted or unquoted string with escaped quotes inside.
	StringWithoutEscapedQuotes,  ///< Quoted or unquoted string without any escaped quotes inside
	UnquotedData,  ///< Unquoted data, no escaped quotes inside.
};


/// Type of the cell value
enum class CellType {
	Empty,
	Double,
	String,
};



/// A helper class for compile-time buffer construction and string unescaping.
template<std::size_t Size>
class CellStringBuffer {
	public:

		/// Constructor. Cleans up the data in cell, creating a buffer of size Size.
		constexpr inline explicit CellStringBuffer(std::string_view cell, bool has_escaped_quotes);

		/// Check if the buffer was successfully created and contains a cleaned up string
		[[nodiscard]] constexpr bool isValid() const noexcept;

		/// Return string view to stored buffer.
		/// The returned view has collapsed consecutive double-quotes inside.
		/// \throw std::out_of_range if buffer is invalid (of insufficient size)
		[[nodiscard]] constexpr std::string_view getStringView() const;

		/// Return string view to stored buffer.
		/// The returned view has collapsed consecutive double-quotes inside.
		/// \return std::nullopt if buffer is invalid (of insufficient size)
		[[nodiscard]] constexpr std::optional<std::string_view> getOptionalStringView() const noexcept;

		/// Return buffer size.
		[[nodiscard]] constexpr std::size_t getBufferSize() const noexcept;


	private:

		struct Buffer {
			std::array<char, Size> buffer = { };
			std::size_t size = 0;
			bool valid = false;
		};

		/// Create a buffer object, with cleaned-up input in it
		[[nodiscard]] constexpr static Buffer prepareBuffer(std::string_view input, bool has_escaped_quotes);

		/// Unescape a string view to newly created buffer
		[[nodiscard]] constexpr static Buffer cleanString(std::string_view input);

		Buffer buffer_;
};



/// A value of a cell.
/// If the cell type is CellType::String, this object references the data in original CSV text.
class CellReference {
	public:

		/// Constructor
		CellReference() = default;

		/// Constructor
		inline CellReference(std::string_view cell, CellTypeHint hint);

		/// Get cell type
		[[nodiscard]] inline CellType getType() const;

		/// Check whether the cell is of CellType::Empty type.
		[[nodiscard]] inline bool isEmpty() const;

		/// Get the cell value as a double.
		/// This succeeds only if cell type is CellType::Double.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<double> getDouble() const;

		/// Get stored cell reference as string_view.
		/// This succeeds only if cell type is CellType::String.
		/// This cell may contain escaped consecutive double-quotes inside; if has_escaped_quotes is not nullptr,
		/// *has_escaped_quotes will reflect that.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<std::string_view> getOriginalStringView(
				bool* has_escaped_quotes = nullptr) const;

		/// Get stored cell reference as string.
		/// This succeeds only if cell type is CellType::String.
		/// The string has collapsed consecutive double-quotes inside.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<std::string> getCleanString() const;

	private:

		/// Empty value (empty unquoted cell)
		struct Empty { };

		/// String value
		struct String {
			std::string_view view;
			bool has_escaped_quotes = false;
		};

		/// Stored data
		std::variant<
			Empty,
			double,
			String
		> value_ = Empty();
};



/// A value of a cell.
/// This object always owns its data and does not reference the original CSV text.
class CellValue {
	public:

		/// Constructor
		CellValue() = default;

		/// Constructor
		inline CellValue(std::string_view cell, CellTypeHint hint);

		/// Get cell type
		[[nodiscard]] inline CellType getType() const;

		/// Check whether the cell is of CellType::Empty type.
		[[nodiscard]] inline bool isEmpty() const;

		/// Get the cell value if cell type is Double.
		/// This succeeds only if cell type is CellType::Double.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<double> getDouble() const;

		/// Get stored cell reference as string.
		/// This succeeds only if cell type is CellType::String.
		/// The string has collapsed consecutive double-quotes inside.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<std::string> getString() const;

	private:

		/// Empty value (empty unquoted cell)
		struct Empty { };

		/// Stored data
		std::variant<
			Empty,
			double,
			std::string
		> value_ = Empty();
};



/// A value of a cell. All cell contents are treated as having CellType::Double type.
/// If conversion failure occurs, NaN is assumed.
/// The data is always owned by this object.
class CellDoubleValue {
	public:

		/// Constructor
		CellDoubleValue() = default;

		/// Constructor
		inline explicit CellDoubleValue(std::string_view cell,
				CellTypeHint hint_ignored = CellTypeHint::UnquotedData);

		/// Get the cell value if cell type is Double.
		/// \return std::numeric_limits<double>::quiet_NaN() on error.
		[[nodiscard]] inline double getValue() const;

	private:
		/// Stored data
		double value_ = std::numeric_limits<double>::quiet_NaN();
};




/// A value of a cell, referencing the data in original CSV text.
/// All cell contents are treated as having CellType::String type.
class CellStringReference {
	public:

		/// Constructor
		constexpr CellStringReference() = default;

		/// Constructor
		inline constexpr CellStringReference(std::string_view cell, CellTypeHint hint) noexcept;

		/// Get stored cell reference as string_view.
		/// Cell type is assumed to be CellType::String, regardless of auto-detected type.
		/// This cell may contain escaped consecutive double-quotes inside; if has_escaped_quotes is not nullptr,
		/// *has_escaped_quotes will reflect that.
		[[nodiscard]] inline constexpr std::string_view getOriginalStringView(
				bool* has_escaped_quotes = nullptr) const noexcept;

		/// Get stored cell reference as string.
		/// Cell type is assumed to be CellType::String, regardless of auto-detected type.
		/// The string has collapsed consecutive double-quotes inside.
		[[nodiscard]] inline std::string getCleanString();

		/// Get a string buffer with collapsed consecutive double-quotes.
		/// This function is useful in constexpr context to retrieve unescaped cell data.
		/// \tparam BufferSize buffer size has to be at least strlen(getOriginalStringView()). Note
		/// that buffer size is always checked, regardless of whether quotes had to be escaped or not.
		/// Reserving additional character for terminating null is not required.
		/// \return invalid buffer if BufferSize is too small.
		template<std::size_t BufferSize>
		[[nodiscard]] constexpr CellStringBuffer<BufferSize> getCleanStringBuffer() const;

		/// Get required buffer size to use as getCleanStringBuffer()'s template argument.
		/// This function is useful in constexpr context.
		[[nodiscard]] constexpr std::size_t getRequiredBufferSize() const;


	private:

		/// Stored data
		std::string_view value_;

		/// If the stored data may contain unescaped double-quotes, this is set to true.
		bool has_escaped_quotes_ = false;

};



/// A value of a cell. The object owns its data and does not reference the original CSV text.
/// All cell contents are treated as strings.
class CellStringValue {
	public:

		/// Constructor
		CellStringValue() = default;

		/// Constructor
		inline CellStringValue(std::string_view cell, CellTypeHint hint);

		/// Get stored cell reference as string.
		/// Cell type is assumed to be String, regardless of autodetected type.
		/// The string has collapsed consecutive double-quotes inside.
		[[nodiscard]] inline const std::string& getString() const;

	private:
		/// Stored data
		std::string value_;
};



/// Parser::parse*() functions use this to create the value_type object of a container.
/// By default, it handles Cell* classes and primitive numeric types.
/// This trait can be specialized for user-defined types.
template<typename CellT>
class CellTrait {
	public:
		/// Create an object of type CellT from cell contents represented as string_view.
		[[nodiscard]] static constexpr CellT create(std::string_view cell, CellTypeHint hint);
};




// ----- Implementation



template<std::size_t Size>
constexpr CellStringBuffer<Size>::CellStringBuffer(std::string_view cell, bool has_escaped_quotes)
		: buffer_(prepareBuffer(cell, has_escaped_quotes))
{ }



template<std::size_t Size>
constexpr bool CellStringBuffer<Size>::isValid() const noexcept
{
	return buffer_.valid;
}



template<std::size_t Size>
constexpr std::string_view CellStringBuffer<Size>::getStringView() const
{
	if (!buffer_.valid) {
		throw std::out_of_range("Insufficient buffer size");
	}
	return {buffer_.buffer.data(), buffer_.size};
}



template<std::size_t Size>
constexpr std::optional<std::string_view> CellStringBuffer<Size>::getOptionalStringView() const noexcept
{
	if (!buffer_.valid) {
		return std::nullopt;
	}
	return std::string_view{buffer_.buffer.data(), buffer_.size};
}



template<std::size_t Size>
constexpr std::size_t CellStringBuffer<Size>::getBufferSize() const noexcept
{
	return Size;
}



template<std::size_t Size>
constexpr typename CellStringBuffer<Size>::Buffer CellStringBuffer<Size>::prepareBuffer(std::string_view input, bool
has_escaped_quotes)
{
	if (Size < input.size()) {
		return Buffer{};
	}
	if (has_escaped_quotes) {
		return cleanString(input);
	}
	std::array<char, Size> buffer = { };
	for (std::size_t pos = 0; pos < std::min(Size, input.size()); ++pos) {
		buffer[pos] = input[pos];
	}
	return Buffer{buffer, input.size(), true};
}



template<std::size_t Size>
constexpr typename CellStringBuffer<Size>::Buffer CellStringBuffer<Size>::cleanString(std::string_view input)
{
	std::array<char, Size> buffer = { };
	std::size_t output_pos = 0;
	for (std::size_t input_pos = 0; input_pos < std::min(Size, input.size()); ++input_pos) {
		char c = input[input_pos];
		buffer[output_pos] = c;
		++output_pos;
		if (c == '\"' && (input_pos + 1) < input.size() && input[input_pos + 1] == '\"') {
			++input_pos;
		}
	}
	return {buffer, output_pos, true};
}





CellReference::CellReference(std::string_view cell, CellTypeHint hint)
{
	switch (hint) {
		case CellTypeHint::Empty:
			// Nothing, value is empty
			break;

		case CellTypeHint::StringWithEscapedQuotes:
			value_ = String{cell, true};
			break;

		case CellTypeHint::StringWithoutEscapedQuotes:
			value_ = String{cell, false};
			break;

		case CellTypeHint::UnquotedData:
			if (auto double_value = readNumber<double>(cell); double_value.has_value()) {
				value_ = double_value.value();
			} else {
				value_ = String{cell, false};
			}
			break;
	}
}



CellType CellReference::getType() const
{
	if (std::holds_alternative<Empty>(value_)) {
		return CellType::Empty;
	}
	if (std::holds_alternative<double>(value_)) {
		return CellType::Double;
	}
	if (std::holds_alternative<String>(value_)) {
		return CellType::String;
	}
	throw std::bad_variant_access();
}



bool CellReference::isEmpty() const
{
	return std::holds_alternative<Empty>(value_);
}



std::optional<double> CellReference::getDouble() const
{
	if (std::holds_alternative<double>(value_)) {
		return std::get<double>(value_);
	}
	return {};
}



std::optional<std::string_view> CellReference::getOriginalStringView(bool* has_escaped_quotes) const
{
	if (std::holds_alternative<String>(value_)) {
		const auto& s = std::get<String>(value_);
		if (has_escaped_quotes) {
			*has_escaped_quotes = s.has_escaped_quotes;
		}
		return s.view;
	}
	return {};
}



std::optional<std::string> CellReference::getCleanString() const
{
	if (std::holds_alternative<String>(value_)) {
		const auto& s = std::get<String>(value_);
		return s.has_escaped_quotes ? cleanString(s.view) : std::string(s.view);
	}
	return {};
}





CellValue::CellValue(std::string_view cell, CellTypeHint hint)
{
	switch (hint) {
		case CellTypeHint::Empty:
			// Nothing, value is empty
			break;

		case CellTypeHint::StringWithEscapedQuotes:
			value_ = cleanString(cell);
			break;

		case CellTypeHint::StringWithoutEscapedQuotes:
			value_ = std::string(cell);
			break;

		case CellTypeHint::UnquotedData:
			if (auto double_value = readNumber<double>(cell); double_value.has_value()) {
				value_ = double_value.value();
			} else {
				value_ = std::string(cell);
			}
			break;
	}
}



CellType CellValue::getType() const
{
	if (std::holds_alternative<Empty>(value_)) {
		return CellType::Empty;
	}
	if (std::holds_alternative<double>(value_)) {
		return CellType::Double;
	}
	if (std::holds_alternative<std::string>(value_)) {
		return CellType::String;
	}
	throw std::bad_variant_access();
}



bool CellValue::isEmpty() const
{
	return std::holds_alternative<Empty>(value_);
}



std::optional<double> CellValue::getDouble() const
{
	if (std::holds_alternative<double>(value_)) {
		return std::get<double>(value_);
	}
	return {};
}



std::optional<std::string> CellValue::getString() const
{
	if (std::holds_alternative<std::string>(value_)) {
		return std::get<std::string>(value_);
	}
	return {};
}





CellDoubleValue::CellDoubleValue(std::string_view cell,
		[[maybe_unused]] CellTypeHint hint_ignored)
{
	if (auto double_value = readNumber<double>(cell); double_value.has_value()) {
		value_ = double_value.value();
	}
}



double CellDoubleValue::getValue() const
{
	return value_;
}





constexpr CellStringReference::CellStringReference(std::string_view cell, CellTypeHint hint) noexcept
		: value_(cell),
		has_escaped_quotes_(hint == CellTypeHint::StringWithEscapedQuotes)
{ }



constexpr std::string_view CellStringReference::getOriginalStringView(bool* has_escaped_quotes) const noexcept
{
	if (has_escaped_quotes) {
		*has_escaped_quotes = has_escaped_quotes_;
	}
	return value_;
}



std::string CellStringReference::getCleanString()
{
	return has_escaped_quotes_ ? cleanString(value_) : std::string(value_);
}



template<std::size_t BufferSize>
constexpr CellStringBuffer<BufferSize> CellStringReference::getCleanStringBuffer() const
{
	return CellStringBuffer<BufferSize>(value_, has_escaped_quotes_);
}



constexpr std::size_t CellStringReference::getRequiredBufferSize() const
{
	return value_.size();
}





CellStringValue::CellStringValue(std::string_view cell, CellTypeHint hint)
		: value_(hint == CellTypeHint::StringWithEscapedQuotes ? cleanString(cell) : std::string(cell))
{ }



const std::string& CellStringValue::getString() const
{
	return value_;
}





template<typename CellT>
constexpr CellT CellTrait<CellT>::create(std::string_view cell, CellTypeHint hint)
{
	// The types here are the same as in readNumber()
	if constexpr(std::is_same_v<CellT, float>
			|| std::is_same_v<CellT, double>
			|| std::is_same_v<CellT, long double>) {
		return readNumber<CellT>(cell).value_or(std::numeric_limits<CellT>::quiet_NaN());
	} else if constexpr(std::is_same_v<CellT, int>
			|| std::is_same_v<CellT, long int>
			|| std::is_same_v<CellT, long long int>
			|| std::is_same_v<CellT, unsigned long int>
			|| std::is_same_v<CellT, unsigned long long int>) {
		return readNumber<CellT>(cell).value_or(0);
	} else {
		// Cell* classes
		return CellT(cell, hint);
	}
}



}  // end ns



#endif
