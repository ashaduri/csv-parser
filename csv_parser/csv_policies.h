/**************************************************************************
Copyright: (C) 2025 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_POLICIES_H
#define CSV_POLICIES_H

#include "csv_cell.h"  // CellTypeHint

#include <optional>
#include <string_view>
#include <type_traits>
#include <limits>


/**
 * \file
 * Behavior policies for Csv::Parser, passed as template parameter.
 */



namespace Csv {


/// This policy controls the behavior of the parser.
/// This version does uses locale-aware number conversion.
/// To override, create a new policy struct (possibly by inheriting from this one)
/// and pass it to the Parser class as a template parameter.
struct LocaleAwareBehaviorPolicy {

	/// If true, empty cell type is a separate type from (empty) string.
	/// Default: true.
	[[nodiscard]] static constexpr bool useEmptyCellType() noexcept
	{
		return true;
	}


	/// Try to read a numeric value from string data.
	/// \see Csv::readNumberLocale().
	template<typename Number>
	static std::optional<Number> readNumber(std::string_view cell)
	{
		return readNumberLocale<Number>(cell);
	}


	/// Create an object of type CellT from cell contents represented as string_view.
	/// By default, it handles Cell* classes (CellReference, etc.) and primitive numeric types (long int, double, etc.).
	template<typename CellT>
	static constexpr CellT create(std::string_view cell, CellTypeHint hint)
	{
		// The types here are the same as in readNumber()
		if constexpr(std::is_arithmetic_v<CellT>) {
			// quiet_NaN() returns 0 for integral types
			return LocaleAwareBehaviorPolicy::readNumber<CellT>(cell)
					.value_or(std::numeric_limits<CellT>::quiet_NaN());
		} else {
			// Cell* classes
			return CellT{cell, hint, LocaleAwareBehaviorPolicy{}};
		}
	}
};



/// This policy controls the behavior of the parser.
/// This version does not use locale-aware number conversion.
/// This policy is faster, but less compatible with pre-C++20/23 compilers
/// due to limited std::from_chars() support. However, it allows compile-time
/// parsing of integral types.
struct LocaleUnawareBehaviorPolicy {

	/// If true, empty cell type is a separate type from (empty) string.
	/// Default: true.
	[[nodiscard]] static constexpr bool useEmptyCellType() noexcept
	{
		return true;
	}


	/// Try to read a numeric value from string data.
	/// \see Csv::readNumberNoLocale().
	template<typename Number>
	static constexpr std::optional<Number> readNumber(std::string_view cell)
	{
		return readNumberNoLocale<Number>(cell);
	}


	/// Create an object of type CellT from cell contents represented as string_view.
	/// By default, it handles Cell* classes (CellReference, etc.) and primitive numeric types (long int, double, etc.).
	template<typename CellT>
	static constexpr CellT create(std::string_view cell, CellTypeHint hint)
	{
		// The types here are the same as in readNumber()
		if constexpr(std::is_arithmetic_v<CellT>) {
			// quiet_NaN() returns 0 for integral types
			return LocaleUnawareBehaviorPolicy::readNumber<CellT>(cell)
					.value_or(std::numeric_limits<CellT>::quiet_NaN());
		} else {
			// Cell* classes
			return CellT{cell, hint, LocaleUnawareBehaviorPolicy{}};
		}
	}
};



}  // end ns



#endif
