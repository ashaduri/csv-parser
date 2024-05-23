
find_package(Doxygen)

set(DOXYGEN_GENERATE_HTML NO)
set(DOXYGEN_GENERATE_MAN YES)

if (NOT DOXYGEN_FOUND)
	message(STATUS "Doxygen not found, cannot build API documentation.")
else()

	set(DOXYGEN_HAVE_DOT NO)
	set(DOXYGEN_GENERATE_LATEX NO)
	set(DOXYGEN_GENERATE_HTML YES)
	set(DOXYGEN_GENERATE_MAN NO)
	set(DOXYGEN_RECURSIVE YES)
	set(DOXYGEN_MARKDOWN_SUPPORT YES)
	set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
	set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")

	# set(DOXYGEN_HTML_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/header.html")
	set(DOXYGEN_HTML_FOOTER "${CMAKE_CURRENT_SOURCE_DIR}/theme/footer.html")
	set(DOXYGEN_HTML_EXTRA_STYLESHEET "${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/that_style.css")
	
	set(DOXYGEN_HTML_EXTRA_FILES "\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/nav_edge_left.svg;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/nav_edge_right.svg;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/nav_edge_inter.svg;\
${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/sync_off.png;\
${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/sync_on.png;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/splitbar_handle.svg;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/doc.svg;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/mag_glass.svg;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/folderclosed.svg;\
#${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/img/folderopen.svg;\
${CMAKE_CURRENT_SOURCE_DIR}/theme/that_style/js/striped_bg.js")

	# Create target "doxygen"
	doxygen_add_docs(
		doxygen
			${PROJECT_SOURCE_DIR}/csv_parser
			${CMAKE_SOURCE_DIR}/README.md
		# ALL  # Add to "All" build target
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generate API documentation"
	)

endif()


# This file is based on:
# https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/

#
## All public headers
#get_target_property(DOC_PUBLIC_HEADER_DIR csv_parser INTERFACE_INCLUDE_DIRECTORIES)
#file(GLOB_RECURSE DOC_PUBLIC_HEADERS ${DOC_PUBLIC_HEADER_DIR}/*.h)
#
## Output file
#set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/csv_parser)
#set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen)
#set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
#set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
#set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
#
#configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
#
#if (NOT DOXYGEN_FOUND)
#	message(STATUS "Doxygen not found, cannot build API documentation.")
#else()
#	file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) # Doxygen won't create this for us
#
#	# Only regenerate Doxygen when the Doxyfile or public headers change
#	add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
#		DEPENDS ${DOC_PUBLIC_HEADERS}
#		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
#		MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
#		COMMENT "Generating API documentation"
#		VERBATIM)
#
#	# Nice named target so we can run the job easily
#	add_custom_target(doxygen_doc ALL DEPENDS ${DOXYGEN_INDEX_FILE})
#endif()
#
#
## Sphinx
#find_program(SPHINX_EXECUTABLE
#             NAMES sphinx-build
#             DOC "Path to sphinx-build executable")
#
#include(FindPackageHandleStandardArgs)
#
##Handle standard arguments to find_package like REQUIRED and QUIET
#find_package_handle_standard_args(Sphinx
#	"Failed to find sphinx-build executable"
#	SPHINX_EXECUTABLE)
#
#
#set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
#set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
#set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
#
## Only regenerate Sphinx when:
## - Doxygen has rerun
## - Our doc files have been updated
## - The Sphinx config has been updated
#add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
#		COMMAND
#		${SPHINX_EXECUTABLE} -b html
#		# Tell Breathe where to find the Doxygen output
#		-Dbreathe_projects.CsvParser=${DOXYGEN_OUTPUT_DIR}/xml
#		${SPHINX_SOURCE} ${SPHINX_BUILD}
#		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#		DEPENDS
#			# Other docs files you want to track should go here (or in some variable)
#			${CMAKE_CURRENT_SOURCE_DIR}/index.rst
#			${DOXYGEN_INDEX_FILE}
#			MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
#		COMMENT "Generating documentation with Sphinx")
#
## Nice named target so we can run the job easily
#add_custom_target(sphinx_doc ALL DEPENDS ${SPHINX_INDEX_FILE})
#
## Add an install target to install the docs
##include(GNUInstallDirs)
##install(DIRECTORY ${SPHINX_BUILD}
##DESTINATION ${CMAKE_INSTALL_DOCDIR})
