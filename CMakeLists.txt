cmake_minimum_required(VERSION 3.9)
project(csv-parser
	VERSION 1.1.0
	DESCRIPTION "Csv::Parser (csv-parser)"
	LANGUAGES CXX
)

# Set the C++ standard to C++23 if available.
# Generally, we require at least C++17, but C++23 is preferred.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED FALSE)


add_library(compiler_warnings INTERFACE)
target_compile_options(compiler_warnings INTERFACE
	$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
		-Wall -Wextra>
	$<$<CXX_COMPILER_ID:MSVC>:
		/W4>
)

# The main library
add_library(csv_parser INTERFACE)
target_sources(csv_parser
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser/csv_cell.h
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser/csv_error.h
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser/csv_matrix.h
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser/csv_parser.h
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser/csv_policies.h
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser/csv_util.h
)
target_include_directories(csv_parser
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/csv_parser
)


# Catch2 target
add_library(catch2 INTERFACE)
target_sources(catch2
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/catch2/catch.hpp
)
target_include_directories(catch2
	SYSTEM INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/catch2
)

# Tests
add_executable(test_csv_parser
	tests/test_csv_parser.cpp
	tests/test_csv_parser_matrix.cpp
	tests/test_csv_cell.cpp
	tests/test_csv_util.cpp
)
target_link_libraries(test_csv_parser
	PRIVATE
		csv_parser
		catch2
		compiler_warnings
)

# Examples
add_executable(example_runtime
	examples/example_runtime.cpp
)
target_link_libraries(example_runtime
	PRIVATE
		csv_parser
		compiler_warnings
)

add_executable(example_runtime_numeric
	examples/example_runtime_numeric.cpp
)
target_link_libraries(example_runtime_numeric
	PRIVATE
		csv_parser
		compiler_warnings
)

add_executable(example_compiletime
	examples/example_compiletime.cpp
)
target_link_libraries(example_compiletime
	PRIVATE
		csv_parser
		compiler_warnings
)

# This example is disabled from build - it's only used as a syntax check for README.md contents
add_executable(readme_examples
	examples/readme_examples.cpp
)
target_link_libraries(readme_examples
	PRIVATE
		csv_parser
		compiler_warnings
)
#set_target_properties(readme_examples PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)


include(CTest)
include(catch2/Catch.cmake)
catch_discover_tests(test_csv_parser)

add_subdirectory (doc)
