/**************************************************************************
Copyright: (C) 2021 Alexander Shaduri
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



namespace Csv {



/// Type hint associated with the cell to determine the type of the cell value
enum class CellTypeHint {
	Empty,  ///< Empty data (no quotes, no whitespace)
	StringWithEscapedQuotes,  ///< Quoted or unquoted string with escaped quotes inside.
	StringWithoutEscapedQuotes,  ///< Quoted or unquoted string without any escaped quotes inside
	UnquotedData,  ///< Unquoted data, no escaped quotes inside.
};


/// Parsed cell type
enum class CellType {
	Empty,
	Double,
	String,
};



/// A value of a cell, referencing the data in original CSV text (if the data is of string type).
class CellReference {
	public:

		/// Constructor
		CellReference() = default;

		/// Constructor
		inline CellReference(std::string_view cell, CellTypeHint hint);

		/// Get cell type
		[[nodiscard]] inline CellType getType() const;

		/// Check whether the cell is of Empty type
		[[nodiscard]] inline bool isEmpty() const;

		/// Get the cell value if cell type is Double.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<double> getDouble() const;

		/// Get stored cell reference as string_view.
		/// This cell may contain escaped consecutive double-quotes inside; if has_escaped_quotes is not nullptr,
		/// *has_escaped_quotes will reflect that.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<std::string_view> getOriginalStringView(
				bool* has_escaped_quotes = nullptr) const;

		/// Get stored cell reference as string.
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



/// A value of a cell. The object owns its data and does not reference the original CSV text.
class CellValue {
	public:

		/// Constructor
		CellValue() = default;

		/// Constructor
		inline CellValue(std::string_view cell, CellTypeHint hint);

		/// Get cell type
		[[nodiscard]] inline CellType getType() const;

		/// Check whether the cell is of Empty type
		[[nodiscard]] inline bool isEmpty() const;

		/// Get the cell value if cell type is Double.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] inline std::optional<double> getDouble() const;

		/// Get stored cell reference as string.
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



/// A value of a cell. All cell contents are treated as doubles. The data is owned by this object.
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
/// All cell contents are treated as strings.
class CellStringReference {
	public:

		/// Constructor
		constexpr CellStringReference() = default;

		/// Constructor
		inline constexpr CellStringReference(std::string_view cell, CellTypeHint hint);

		/// Get stored cell reference as string_view.
		/// This cell may contain escaped consecutive double-quotes inside; if has_escaped_quotes is not nullptr,
		/// *has_escaped_quotes will reflect that.
		/// \return default-initialized string_view if cell type is not String.
		[[nodiscard]] inline constexpr std::string_view getOriginalStringView(
				bool* has_escaped_quotes = nullptr) const;

		/// Get stored cell reference as string.
		/// The string has collapsed consecutive double-quotes inside.
		[[nodiscard]] inline std::string getCleanString();

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
		/// The string has collapsed consecutive double-quotes inside.
		[[nodiscard]] inline const std::string& getString() const;

	private:
		/// Stored data
		std::string value_;
};





/// Unescape a string - collapse every occurrence of 2 consecutive double-quotes to one.
inline std::string cleanString(std::string_view view);


/// Try to read a double value from string data.
/// Unless the string data (with optional whitespace on either or both sides) completely represents a serialized
/// double, std::nullopt is returned.
inline std::optional<double> readDouble(std::string_view cell);





// ----- Implementation




CellReference::CellReference(std::string_view cell, CellTypeHint hint)
{
	switch (hint) {
		case CellTypeHint::Empty:
			// Nothing, value is empty
			break;

		case CellTypeHint::StringWithEscapedQuotes:
			value_ = String{.view = cell, .has_escaped_quotes = true};
			break;

		case CellTypeHint::StringWithoutEscapedQuotes:
			value_ = String{.view = cell, .has_escaped_quotes = false};
			break;

		case CellTypeHint::UnquotedData:
			if (auto double_value = readDouble(cell); double_value.has_value()) {
				value_ = double_value.value();
			} else {
				value_ = String{.view = cell, .has_escaped_quotes = false};
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
			if (auto double_value = readDouble(cell); double_value.has_value()) {
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
	if (auto double_value = readDouble(cell); double_value.has_value()) {
		value_ = double_value.value();
	}
}



double CellDoubleValue::getValue() const
{
	return value_;
}





constexpr CellStringReference::CellStringReference(std::string_view cell, CellTypeHint hint)
		: value_(cell),
		has_escaped_quotes_(hint == CellTypeHint::StringWithEscapedQuotes)
{ }



constexpr std::string_view CellStringReference::getOriginalStringView(bool* has_escaped_quotes) const
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





CellStringValue::CellStringValue(std::string_view cell, CellTypeHint hint)
		: value_(hint == CellTypeHint::StringWithEscapedQuotes ? cleanString(cell) : std::string(cell))
{ }



const std::string& CellStringValue::getString() const
{
	return value_;
}





std::string cleanString(std::string_view view)
{
	std::string s;
	s.reserve(view.size());
	for (std::size_t pos = 0; pos < view.size(); ++pos) {
		char c = view[pos];
		s += c;
		if (c == '\"' && (pos + 1) < view.size() && view[pos + 1] == '\"') {
			++pos;
		}
	}
	return s;
}



std::optional<double> readDouble(std::string_view cell)
{
	// Trim right whitespace (left whitespace is ignored by stod()).
	std::size_t size = cell.size();
	if (auto end_pos = cell.find_last_not_of(" \t"); end_pos != std::string_view::npos) {
		size = end_pos + 1;
	}
	std::string s(cell.data(), size);

	// Convert to lowercase (needed for Matlab-produced CSV files)
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});

	std::optional<double> double_value;
	// As of 2020, from_chars() is broken for floats/doubles in most compilers, so we'll have to do with stod() for
	// now, even if it means using current locale instead of C locale.
	// While calling std::strtod() could be potentially faster, it also means we have to deal with some
	// platform-specific errno and other peculiarities. std::stod() wraps that nicely.
	try {
		std::size_t num_processed = 0;
		// We have to use a 0-terminated string in stod().
		double parsed_double = std::stod(s, &num_processed);
		if (num_processed == s.size()) {
			double_value = parsed_double;
		}
	} catch (...) {
		// nothing
	}
	return double_value;
}



}  // end ns



#endif
