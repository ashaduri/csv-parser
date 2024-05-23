/**************************************************************************
Copyright: (C) 2022 Alexander Shaduri
License: Zlib
***************************************************************************/

#ifndef CSV_MATRIX_H
#define CSV_MATRIX_H

#include <cstddef>


/**
 * \file
 * Matrix-related classes used when parsing CSV data into matrices.
*/



namespace Csv {



/// Order of elements in a matrix.
enum class MatrixOrder {
	RowMajor,  ///< A11, A12, A13, A21, ...
	ColumnMajor,  ///< A11, A21, A31, A12, ...
};



/// Matrix information (dimensions, order).
class MatrixInformation {
	public:

		/// Get index (offset) in flat-matrix vector.
		/// \param row 0-based row number
		/// \param column 0-based column number
		/// \param rows Number of rows in matrix
		/// \param columns Number of columns in matrix
		/// \param order Matrix order
		/// \return 0-based index in vector
		[[nodiscard]] static constexpr std::size_t matrixIndex(std::size_t row, std::size_t column,
				std::size_t rows, std::size_t columns, MatrixOrder order)
		{
			if (order == MatrixOrder::RowMajor) {
				return row * columns + column;
			}
			return column * rows + row;
		}



		/// Get index (offset) in flat-matrix vector, based on matrix information.
		/// \param row 0-based row number
		/// \param column 0-based column number
		/// \return 0-based index in vector
		[[nodiscard]] constexpr std::size_t matrixIndex(std::size_t row, std::size_t column) const
		{
			return matrixIndex(row, column, rows_, columns_, order_);
		}


		/// Get number of rows in matrix.
		[[nodiscard]] constexpr std::size_t getRows() const
		{
			return rows_;
		}


		/// Set number of rows in matrix.
		constexpr void setRows(const std::size_t rows)
		{
			rows_ = rows;
		}


		/// Get number of columns in matrix.
		[[nodiscard]] constexpr std::size_t getColumns() const
		{
			return columns_;
		}


		/// Set number of columns in matrix.
		constexpr void setColumns(const std::size_t columns)
		{
			columns_ = columns;
		}


		/// Get element order (row-major or column-major).
		[[nodiscard]] constexpr MatrixOrder getOrder() const
		{
			return order_;
		}


		/// Set element order (row-major or column-major).
		constexpr void setOrder(MatrixOrder order)
		{
			order_ = order;
		}


	private:
		std::size_t rows_ = 0;   ///< Number of rows in matrix
		std::size_t columns_ = 0;  ///< Number of columns in matrix
		MatrixOrder order_ = MatrixOrder::RowMajor;  ///< Element order

};



}  // end ns



#endif
