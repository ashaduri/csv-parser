/**************************************************************************
Copyright: (C) 2021 - 2025 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_CELL_H
#define CSV_CELL_H

#include "csv_util.h"
#include "csv_cell_string_buffer.h"

#include <string_view>
#include <string>
#include <optional>
#include <algorithm>
#include <variant>
#include <limits>
#include <stdexcept>
#include <array>

/**
 * \file
 * Various Cell-related classes.
 */



namespace Csv {


/// Type hint associated with the cell to determine the type of the cell value
enum class CellTypeHint {
	Empty,  ///< Empty data (no quotes, no whitespace).
	StringWithEscapedQuotes,  ///< Quoted or unquoted string with escaped quotes inside.
	StringWithoutEscapedQuotes,  ///< Quoted or unquoted string without any escaped quotes inside.
	UnquotedData,  ///< Unquoted data, no escaped quotes inside.
};


/// Type of the cell value
enum class CellType {
	Empty,  ///< Empty cell, no data
	Double,  ///< `double` value
	String,  ///< String data
};



/// Data inside a single cell, potentially stored as a reference to the original data.
/// If the cell type is CellType::String, this object references the data in original CSV text.
class CellReference {
	public:

		/// Constructor, creates an Empty value.
		constexpr CellReference() = default;

		/// Constructor, which stores a copy or a reference to the original the cell data.
		/// \param cell Cell data to store or reference.
		/// \param hint Type hint associated with the cell during parsing.
		/// \param policy Argument used for type inference.
		/// \tparam BehaviorPolicy Policy to use for parsing, same as BehaviorPolicy in Parser.
		template<typename BehaviorPolicy>
		constexpr CellReference(std::string_view cell, CellTypeHint hint, BehaviorPolicy policy) noexcept;

		/// Get data type
		[[nodiscard]] inline constexpr CellType getType() const;

		/// Check whether the data is of CellType::Empty type.
		[[nodiscard]] inline constexpr bool isEmpty() const;

		/// Get the cell value as a double.
		/// This succeeds only if cell type is CellType::Double.
		/// \return `std::nullopt` on type mismatch.
		[[nodiscard]] inline constexpr std::optional<double> getDouble() const;

		/// Get stored data reference as string_view to the original CSV data.
		/// This succeeds only if cell type is CellType::String.
		/// \param[out] has_escaped_quotes This cell may contain escaped consecutive double-quotes inside;
		/// if has_escaped_quotes is not `std::nullptr`, *has_escaped_quotes will reflect that.
		/// \return `std::nullopt` on type mismatch.
		[[nodiscard]] inline constexpr std::optional<std::string_view> getOriginalStringView(
				bool* has_escaped_quotes = nullptr) const;

		/// Get stored cell data as `std::string`.
		/// This succeeds only if cell type is CellType::String.
		/// The string has collapsed consecutive double-quotes inside.
		/// \return `std::nullopt` on type mismatch.
		[[nodiscard]] inline std::optional<std::string> getCleanString() const;

		/// Get a string buffer with collapsed consecutive double-quotes.
		/// This function is useful in constexpr context to retrieve unescaped cell data.
		/// \tparam BufferSize buffer size has to be at least `getOriginalStringView().size()`. Note
		/// that buffer size is always checked, regardless of whether quotes had to be escaped or not.
		/// Reserving additional character for terminating null is not required.
		/// \return invalid buffer if BufferSize is too small.
		template<std::size_t BufferSize>
		[[nodiscard]] constexpr CellStringBuffer<BufferSize> getCleanStringBuffer() const;

		/// Get required buffer size to use as getCleanStringBuffer()'s template argument.
		/// This function is useful in constexpr context.
		[[nodiscard]] constexpr std::size_t getRequiredBufferSize() const;


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



/// Data inside a single cell, stored as a copy.
/// This object always owns its data and does not reference the original CSV text.
class CellValue {
	public:

		/// Constructor, creates an Empty value.
		CellValue() = default;

		/// Constructor, which stores a copy of the cell data.
		/// \param cell Cell data to store.
		/// \param hint Type hint associated with the cell during parsing.
		/// \param policy Argument used for type inference.
		/// \tparam BehaviorPolicy Policy to use for parsing, same as BehaviorPolicy in Parser.
		template<typename BehaviorPolicy>
		CellValue(std::string_view cell, CellTypeHint hint, BehaviorPolicy policy);

		/// Get cell type
		[[nodiscard]] inline CellType getType() const;

		/// Check whether the cell is of CellType::Empty type.
		[[nodiscard]] inline bool isEmpty() const;

		/// Get the cell value if cell type is Double.
		/// This succeeds only if cell type is CellType::Double.
		/// \return `std::nullopt` on type mismatch.
		[[nodiscard]] inline std::optional<double> getDouble() const;

		/// Get stored cell value as `std::string`.
		/// This succeeds only if cell type is CellType::String.
		/// The string has collapsed consecutive double-quotes inside.
		/// \return `std::nullopt` on type mismatch.
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



/// Data inside a single cell, stored as a value of `double` type.
/// All cell contents are treated as having CellType::Double type, regardless of its original type.
/// The data is always owned by this object.
/// This may be useful if the CSV has numeric data in double-quotes.
class CellDoubleValue {
	public:

		/// Constructor, setting the value to `std::numeric_limits<double>::quiet_NaN()`.
		constexpr CellDoubleValue() = default;

		/// Constructor, which stores a copy of the cell data.
		/// If conversion failure occurs, NaN is assumed.
		/// \param cell Cell data to store.
		/// \param hint_ignored This parameter is ignored and is present for compatibility with CellTrait::create().
		/// \param policy Argument used for type inference.
		/// \tparam BehaviorPolicy Policy to use for parsing, same as BehaviorPolicy in Parser.
		template<typename BehaviorPolicy>
		constexpr explicit CellDoubleValue(std::string_view cell,
				CellTypeHint hint_ignored, BehaviorPolicy policy) noexcept;

		/// Get the cell value if cell type is Double.
		/// \return `std::numeric_limits<double>::quiet_NaN()` on error.
		[[nodiscard]] inline constexpr double getValue() const;

	private:
		/// Stored data
		double value_ = std::numeric_limits<double>::quiet_NaN();
};




/// String data inside a single cell, stored as a reference to the original data.
/// All cell contents are treated as having CellType::String type.
/// This may be useful if the CSV has string data without quotes.
/// \note: Some members of this class are suitable for constexpr context (compile-time parsing).
class CellStringReference {
	public:

		/// Constructor, creates an empty string reference.
		constexpr CellStringReference() = default;

		/// Constructor, which stores a reference to the original cell data.
		/// \param cell Cell data to reference.
		/// \param hint Type hint associated with the cell during parsing.
		/// \param policy Argument used for type inference.
		/// \tparam BehaviorPolicy Policy to use for parsing, same as BehaviorPolicy in Parser.
		template<typename BehaviorPolicy>
		constexpr CellStringReference(std::string_view cell, CellTypeHint hint, BehaviorPolicy policy) noexcept;

		/// Get stored cell reference as string_view.
		/// Cell type is assumed to be CellType::String, regardless of auto-detected type.
		/// \param[out] has_escaped_quotes This cell may contain escaped consecutive double-quotes inside;
		/// if has_escaped_quotes is not `std::nullptr`, *has_escaped_quotes will reflect that.
		[[nodiscard]] inline constexpr std::string_view getOriginalStringView(
				bool* has_escaped_quotes = nullptr) const noexcept;

		/// Get stored cell reference a `std::string`.
		/// Cell type is assumed to be CellType::String, regardless of auto-detected type.
		/// The string has collapsed consecutive double-quotes inside.
		/// \return empty string on type mismatch.
		[[nodiscard]] inline std::string getCleanString();

		/// Get a string buffer with collapsed consecutive double-quotes.
		/// This function is useful in constexpr context to retrieve unescaped cell data.
		/// \tparam BufferSize buffer size has to be at least `getOriginalStringView().size()`. Note
		/// that buffer size is always checked, regardless of whether quotes had to be escaped or not.
		/// Reserving additional character for terminating null is not required.
		/// \return invalid buffer if BufferSize is too small.
		template<std::size_t BufferSize>
		[[nodiscard]] constexpr CellStringBuffer<BufferSize> getCleanStringBuffer() const;

		/// Get required buffer size to use as getCleanStringBuffer()'s template argument.
		/// This function is useful in constexpr context.
		[[nodiscard]] constexpr std::size_t getRequiredBufferSize() const;


	private:

		/// Stored reference to original data.
		std::string_view value_;

		/// If the stored data may contain unescaped double-quotes, this is set to true.
		bool has_escaped_quotes_ = false;

};



/// A value of a cell. The object owns its data and does not reference the original CSV text.
/// All cell contents are treated as strings.
/// This may be useful if the CSV has string data without quotes.
class CellStringValue {
	public:

		/// Constructor
		CellStringValue() = default;

		/// Constructor, which stores a copy of the cell data.
		/// \param cell Cell data to store.
		/// \param hint Type hint associated with the cell during parsing.
		/// \param policy Argument used for type inference.
		/// \tparam BehaviorPolicy Policy to use for parsing, same as BehaviorPolicy in Parser.
		template<typename BehaviorPolicy>
		CellStringValue(std::string_view cell, CellTypeHint hint, BehaviorPolicy policy);

		/// Get stored cell reference as string.
		/// Cell type is assumed to be String, regardless of autodetected type.
		/// The string has collapsed consecutive double-quotes inside.
		[[nodiscard]] inline const std::string& getString() const;

	private:
		/// Stored data
		std::string value_;
};




// ----- Implementation



template<typename BehaviorPolicy>
constexpr CellReference::CellReference(std::string_view cell, CellTypeHint hint,
		[[maybe_unused]] BehaviorPolicy policy) noexcept
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
			if (auto double_value = BehaviorPolicy::template readNumber<double>(cell); double_value.has_value()) {
				value_ = double_value.value();
			} else {
				value_ = String{cell, false};
			}
			break;
	}
}



constexpr CellType CellReference::getType() const
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



constexpr bool CellReference::isEmpty() const
{
	return std::holds_alternative<Empty>(value_);
}



constexpr std::optional<double> CellReference::getDouble() const
{
	if (std::holds_alternative<double>(value_)) {
		return std::get<double>(value_);
	}
	return {};
}



constexpr std::optional<std::string_view> CellReference::getOriginalStringView(bool* has_escaped_quotes) const
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



template<std::size_t BufferSize>
constexpr CellStringBuffer<BufferSize> CellReference::getCleanStringBuffer() const
{
	// This throws bad variant access
	const auto& s = std::get<String>(value_);
	return CellStringBuffer<BufferSize>(s.view, s.has_escaped_quotes);
}



constexpr std::size_t CellReference::getRequiredBufferSize() const
{
	// This throws bad variant access
	const auto& s = std::get<String>(value_);
	return s.has_escaped_quotes ? getCleanStringSize(s.view) : s.view.size();
}





template<typename BehaviorPolicy>
CellValue::CellValue(std::string_view cell, CellTypeHint hint, [[maybe_unused]] BehaviorPolicy policy)
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
			if (auto double_value = BehaviorPolicy::template readNumber<double>(cell); double_value.has_value()) {
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





template<typename BehaviorPolicy>
constexpr CellDoubleValue::CellDoubleValue(std::string_view cell,
		[[maybe_unused]] CellTypeHint hint_ignored, [[maybe_unused]] BehaviorPolicy policy) noexcept
{
	if (auto double_value = BehaviorPolicy::template readNumber<double>(cell); double_value.has_value()) {
		value_ = double_value.value();
	}
}



constexpr double CellDoubleValue::getValue() const
{
	return value_;
}





template<typename BehaviorPolicy>
constexpr CellStringReference::CellStringReference(std::string_view cell, CellTypeHint hint,
		[[maybe_unused]] BehaviorPolicy policy) noexcept
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
	return getCleanStringSize(value_);
}





template<typename BehaviorPolicy>
CellStringValue::CellStringValue(std::string_view cell, CellTypeHint hint,
		[[maybe_unused]] BehaviorPolicy policy)
		: value_(hint == CellTypeHint::StringWithEscapedQuotes ? cleanString(cell) : std::string(cell))
{ }



const std::string& CellStringValue::getString() const
{
	return value_;
}




}  // end ns



#endif
