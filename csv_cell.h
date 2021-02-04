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


/// Type hint associated with the cell to determine the type of the cell value
enum class CsvCellTypeHint {
	Empty,
	Quoted,
	Unquoted,
};


/// Parsed cell type
enum class CsvCellType {
	Empty,
	Double,
	String,
};



/// A value of a cell, possibly referencing the data in CSV text.
class CsvCellReference {
	public:

		/// Constructor
		CsvCellReference() = default;

		/// Constructor
		CsvCellReference(std::string_view cell, CsvCellTypeHint hint);

		/// Get cell type
		[[nodiscard]] CsvCellType getType() const;

		/// Check whether the cell is of Empty type
		[[nodiscard]] bool isEmpty() const;

		/// Get the cell value if cell type is Double.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] std::optional<double> getDouble() const;

		/// Get stored cell reference as string_view.
		/// This cell may (or may not) contain the original two consecutive double-quotes.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] std::optional<std::string_view> getOriginalStringView() const;

		/// Get stored cell reference as string.
		/// The string has collapsed consecutive double quotes inside.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] std::optional<std::string> getCleanString() const;

	private:

		/// Empty value (empty unquoted cell)
		struct Empty { };

		std::variant<
			Empty,
			double,
			std::string_view
		> value_ = Empty();
};



/// A value of a cell, owns its data
class CsvCellValue {
	public:

		/// Constructor
		CsvCellValue() = default;

		/// Constructor
		CsvCellValue(std::string_view cell, CsvCellTypeHint hint);

		/// Get cell type
		[[nodiscard]] CsvCellType getType() const;

		/// Check whether the cell is of Empty type
		[[nodiscard]] bool isEmpty() const;

		/// Get the cell value if cell type is Double.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] std::optional<double> getDouble() const;

		/// Get stored cell reference as string.
		/// The string has collapsed consecutive double quotes inside.
		/// \return std::nullopt on type mismatch
		[[nodiscard]] std::optional<std::string> getString() const;

	private:

		/// Empty value (empty unquoted cell)
		struct Empty { };

		std::variant<
			Empty,
			double,
			std::string
		> value_ = Empty();
};



/// A value of a cell, owns its data. All cell contents are treated as doubles.
class CsvCellDoubleValue {
	public:

		/// Constructor
		CsvCellDoubleValue() = default;

		/// Constructor
		explicit CsvCellDoubleValue(std::string_view cell, [[maybe_unused]] CsvCellTypeHint hint_ignored = CsvCellTypeHint::Empty);

		/// Get the cell value if cell type is Double.
		/// \return std::numeric_limits<double>::quiet_NaN() on error.
		[[nodiscard]] double getValue() const;

	private:
		double value_ = std::numeric_limits<double>::quiet_NaN();
};



/// A value of a cell, referencing the data in CSV text.
/// All cell contents are treated as strings.
class CsvCellStringReference {
	public:

		/// Constructor
		constexpr CsvCellStringReference() = default;

		/// Constructor
		constexpr explicit CsvCellStringReference(std::string_view cell, [[maybe_unused]] CsvCellTypeHint hint_ignored = CsvCellTypeHint::Empty);

		/// Get stored cell reference as string_view.
		/// This cell may (or may not) contain the original two consecutive double-quotes.
		/// \return default-initialized string_view if cell type is not String.
		[[nodiscard]] constexpr std::string_view getOriginalStringView() const;

		/// Get stored cell reference as string.
		/// The string has collapsed consecutive double quotes inside.
		[[nodiscard]] std::string getCleanString();

	private:
		std::string_view value_;
};



/// A value of a cell, owning its data.
/// All cell contents are treated as strings.
class CsvCellStringValue {
	public:

		/// Constructor
		CsvCellStringValue() = default;

		/// Constructor
		explicit CsvCellStringValue(std::string_view cell, [[maybe_unused]] CsvCellTypeHint hint_ignored = CsvCellTypeHint::Empty);

		/// Get stored cell reference as string.
		/// The string has collapsed consecutive double quotes inside.
		[[nodiscard]] const std::string& getString() const;

	private:
		std::string value_;
};



// ----- Implementation



namespace csv_detail {


inline std::optional<double> readDouble(std::string_view cell)
{
	// As of 2020, from_chars() is broken for floats in most compilers, so we'll have to do with stod() for now,
	// even if it means using current locale.

	// Trim right whitespace (left whitespace is ignored by stod()).
	std::size_t size = cell.size();
	if (auto end_pos = cell.find_last_not_of(" \t"); end_pos != std::string_view::npos) {
		size = end_pos + 1;
	}
	std::string s(cell.data(), size);

	// Convert to lowercase (needed for Matlab-produced CSV files)
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return std::tolower(c);
	});

	std::optional<double> double_value;
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


/// Collapse every occurrence of 2 consecutive double-quotes to one.
inline std::string cleanString(std::string_view view)
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


}





CsvCellReference::CsvCellReference(std::string_view cell, CsvCellTypeHint hint)
{
	switch (hint) {
		case CsvCellTypeHint::Empty:
			// Nothing, value is empty
			break;

		case CsvCellTypeHint::Quoted:
			// Assume all quoted cells are strings
			value_ = cell;
			break;

		case CsvCellTypeHint::Unquoted:
			if (auto double_value = csv_detail::readDouble(cell); double_value.has_value()) {
				value_ = double_value.value();
			} else {
				value_ = cell;
			}
			break;
	}
}



CsvCellType CsvCellReference::getType() const
{
	if (std::holds_alternative<Empty>(value_)) {
		return CsvCellType::Empty;
	}
	if (std::holds_alternative<double>(value_)) {
		return CsvCellType::Double;
	}
	if (std::holds_alternative<std::string_view>(value_)) {
		return CsvCellType::String;
	}
	throw std::bad_variant_access();
}



bool CsvCellReference::isEmpty() const
{
	return std::holds_alternative<Empty>(value_);
}



std::optional<double> CsvCellReference::getDouble() const
{
	if (std::holds_alternative<double>(value_)) {
		return std::get<double>(value_);
	}
	return {};
}



std::optional<std::string_view> CsvCellReference::getOriginalStringView() const
{
	if (std::holds_alternative<std::string_view>(value_)) {
		return std::get<std::string_view>(value_);
	}
	return {};
}



std::optional<std::string> CsvCellReference::getCleanString() const
{
	if (std::holds_alternative<std::string_view>(value_)) {
		return csv_detail::cleanString(std::get<std::string_view>(value_));
	}
	return {};
}





CsvCellValue::CsvCellValue(std::string_view cell, CsvCellTypeHint hint)
{
	switch (hint) {
		case CsvCellTypeHint::Empty:
			// Nothing, value is empty
			break;

		case CsvCellTypeHint::Quoted:
			// Assume all quoted cells are strings
			value_ = csv_detail::cleanString(cell);
			break;

		case CsvCellTypeHint::Unquoted:
			if (auto double_value = csv_detail::readDouble(cell); double_value.has_value()) {
				value_ = double_value.value();
			} else {
				value_ = csv_detail::cleanString(cell);
			}
			break;
	}
}



CsvCellType CsvCellValue::getType() const
{
	if (std::holds_alternative<Empty>(value_)) {
		return CsvCellType::Empty;
	}
	if (std::holds_alternative<double>(value_)) {
		return CsvCellType::Double;
	}
	if (std::holds_alternative<std::string>(value_)) {
		return CsvCellType::String;
	}
	throw std::bad_variant_access();
}



bool CsvCellValue::isEmpty() const
{
	return std::holds_alternative<Empty>(value_);
}



std::optional<double> CsvCellValue::getDouble() const
{
	if (std::holds_alternative<double>(value_)) {
		return std::get<double>(value_);
	}
	return {};
}



std::optional<std::string> CsvCellValue::getString() const
{
	if (std::holds_alternative<std::string>(value_)) {
		return std::get<std::string>(value_);
	}
	return {};
}





CsvCellDoubleValue::CsvCellDoubleValue(std::string_view cell, [[maybe_unused]] CsvCellTypeHint hint_ignored)
{
	if (auto double_value = csv_detail::readDouble(cell); double_value.has_value()) {
		value_ = double_value.value();
	}
}



double CsvCellDoubleValue::getValue() const
{
	return value_;
}





constexpr CsvCellStringReference::CsvCellStringReference(std::string_view cell, [[maybe_unused]] CsvCellTypeHint hint_ignored)
{
	value_ = cell;
}



constexpr std::string_view CsvCellStringReference::getOriginalStringView() const
{
	return value_;
}



std::string CsvCellStringReference::getCleanString()
{
	return csv_detail::cleanString(value_);
}





CsvCellStringValue::CsvCellStringValue(std::string_view cell, [[maybe_unused]] CsvCellTypeHint hint_ignored)
{
	value_ = csv_detail::cleanString(cell);
}



const std::string& CsvCellStringValue::getString() const
{
	return value_;
}





#endif
