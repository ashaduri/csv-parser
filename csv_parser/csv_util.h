/**************************************************************************
Copyright: (C) 2021 - 2025 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_UTIL_H
#define CSV_UTIL_H

#include <string_view>
#include <string>
#include <optional>
#include <algorithm>
#include <charconv>


/**
 * \file
 * Utility functions used by the library.
 */


namespace Csv {


namespace internal {

	/// Helper for static_assert
	/// \private
	template<typename T>
	struct always_false : std::false_type {};

}



/// Unescape a string - collapse every occurrence of 2 consecutive double-quotes to one.
/// \param view Input string
/// \return Unescaped string
inline std::string cleanString(std::string_view view);


/// Get size of a cleaned-up string given the uncollapsed string.
/// \param uncollapsed_view Input string
/// \return Size of the cleaned-up string
inline constexpr std::size_t getCleanStringSize(std::string_view uncollapsed_view);


/// Try to read a numeric value from string data.
/// Unless the string data completely represents a serialized int/float/double/..., `std::nullopt` is returned.
/// \note This function is locale-dependent.
/// \tparam Number Type of number to read, e.g. int, float, double, etc.
/// \param cell String data to parse, with optional whitespace on one or both sides.
/// \return Parsed number or `std::nullopt` if parsing failed.
template<typename Number>
std::optional<Number> readNumberLocale(std::string_view cell);


/// Try to read a numeric value from string data.
/// Unless the string data completely represents a serialized int/float/double/..., `std::nullopt` is returned.
/// \note This function is locale-independent.
/// \note This function is constexpr in C++23+ when parsing integral types.
/// \tparam Number Type of number to read, e.g. int, float, double, etc.
/// \param cell String data to parse, with optional whitespace on one or both sides.
/// \return Parsed number or `std::nullopt` if parsing failed.
template<typename Number>
#if __cplusplus >= 202300L
constexpr
#endif
std::optional<Number> readNumberNoLocale(std::string_view cell);


/// A helper function to get an element of a 2D vector in less error-prone way.
/// \tparam Vector2D A 2D vector type, deduced from the argument.
/// \param values A vector of columns
/// \param row 0-based row number
/// \param column 0-based column number
/// \return Value of Vector2D's innermost type
template<typename Vector2D>
static constexpr auto vector2DValue(const Vector2D& values, std::size_t row, std::size_t column);




// ----- Implementation



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



constexpr std::size_t getCleanStringSize(std::string_view uncollapsed_view)
{
	// Count the number of quotes in the string. Since quotes are doubled,
	// subtract half.
	// std::count is not constexpr in C++17, so we have to do it manually.
	std::size_t quote_count = 0;
	for (char c : uncollapsed_view) {
		if (c == '\"') {
			++quote_count;
		}
	}
	return uncollapsed_view.size() - (quote_count / 2);
}



template<typename Number>
std::optional<Number> readNumberLocale(std::string_view cell)
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

	std::optional<Number> numeric_value;
	// Use locale-aware conversion functions to parse the number.
	// While calling std::strtod() could be potentially faster, it also means we have to deal with some
	// platform-specific errno and other peculiarities. std::stod() wraps that nicely.
	try {
		std::size_t num_processed = 0;
		Number parsed_value;

		// We have to use a 0-terminated string in std::sto*() functions.
		if constexpr(std::is_same_v<Number, float>) {
			parsed_value = std::stof(s, &num_processed);
		} else if constexpr(std::is_same_v<Number, double>) {
			parsed_value = std::stod(s, &num_processed);
		} else if constexpr(std::is_same_v<Number, long double>) {
			parsed_value = std::stold(s, &num_processed);

		} else if constexpr(std::is_same_v<Number, int>) {
			parsed_value = std::stoi(s, &num_processed);
		} else if constexpr(std::is_same_v<Number, long int>) {
			parsed_value = std::stol(s, &num_processed);
		} else if constexpr(std::is_same_v<Number, long long int>) {
			parsed_value = std::stoll(s, &num_processed);

		} else if constexpr(std::is_same_v<Number, unsigned long int>) {
			parsed_value = std::stoul(s, &num_processed);
		} else if constexpr(std::is_same_v<Number, unsigned long long int>) {
			parsed_value = std::stoull(s, &num_processed);

		} else {
			static_assert(internal::always_false<Number>::value, "Invalid Number type in Csv::readNumber()");
		}

		if (num_processed == s.size()) {
			numeric_value = parsed_value;
		}
	} catch (...) {
		// nothing
	}
	return numeric_value;
}



template<typename Number>
#if __cplusplus >= 202300L
constexpr
#endif
std::optional<Number> readNumberNoLocale(std::string_view cell)
{
	// Trim right and left whitespace
	cell.remove_prefix(std::min(cell.find_first_not_of(" \t"), cell.size()));
	cell.remove_suffix(std::min(cell.size() - cell.find_last_not_of(" \t") - 1, cell.size()));

	// string can be used in constexpr functions since C++23
	std::string s(cell);

	// Convert to lowercase (needed for Matlab-produced CSV files)
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
		return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;  // tolower is not constexpr
	});

	Number parsed_value = 0;

	// from_chars is constexpr since C++23, but only for integers
	auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), parsed_value);
	if (ec == std::errc() && ptr == (s.data() + s.size())) {
		return parsed_value;
	}
	return std::nullopt;
}



template<typename Vector2D>
constexpr auto vector2DValue(const Vector2D& values, std::size_t row, std::size_t column)
{
	return values.at(column).at(row);
}



}  // end ns



#endif
