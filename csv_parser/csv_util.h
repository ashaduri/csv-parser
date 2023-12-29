/**************************************************************************
Copyright: (C) 2021 - 2023 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_UTIL_H
#define CSV_UTIL_H

#include <string_view>
#include <string>
#include <optional>
#include <algorithm>



namespace Csv {


/// Helper for static_assert
template<typename T>
struct always_false : std::false_type {};


/// Unescape a string - collapse every occurrence of 2 consecutive double-quotes to one.
inline std::string cleanString(std::string_view view);


/// Try to read a numeric value from string data.
/// Unless the string data (with optional whitespace on either or both sides) completely represents a serialized
/// int/float/double/..., std::nullopt is returned.
template<typename Number>
std::optional<Number> readNumber(std::string_view cell);


/// A helper function to get an element of parsed 2D vector in less error-prone way.
/// \tparam Vector2D a vector of columns
/// \return Vector2D's innermost type
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



template<typename Number>
std::optional<Number> readNumber(std::string_view cell)
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
	// As of 2020, from_chars() is broken for floats/doubles in most compilers, so we'll have to do with stod()/... for
	// now, even if it means using current locale instead of C locale.
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
			static_assert(always_false<Number>::value, "Invalid Number type in Csv::readNumber()");
		}

		if (num_processed == s.size()) {
			numeric_value = parsed_value;
		}
	} catch (...) {
		// nothing
	}
	return numeric_value;
}



template<typename Vector2D>
constexpr auto vector2DValue(const Vector2D& values, std::size_t row, std::size_t column)
{
	return values.at(column).at(row);
}



}  // end ns



#endif
