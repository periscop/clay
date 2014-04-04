cmake_minimum_required(VERSION 2.8)


set(PACKAGE_VERSION "0.0.1")
set(top_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# User's settings - C Flags

# 	set(release "TRUE")
	set(release "FALSE")

	# Release
	if (release)
		set(CMAKE_C_FLAGS "-O3")
	# Debug # valgrind --show-reachable=yes --leak-check=full -v exe
	else()
		set(CMAKE_C_FLAGS "-O0 -g3")
	endif()

# User's settings - General C Flags
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99 -pedantic")


# Build doxygen
	find_package(Doxygen)
	if(DOXYGEN_FOUND)
		configure_file("doc/Doxyfile.in" "Doxyfile")
		add_custom_target(
			doxygen
			${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
			COMMENT "Generating API documentation with Doxygen" VERBATIM
		)
	else()
		message (STATUS "Doxygen not found :( API documentation can not be built")
	endif()

# Build documentation

	# doc
	find_program(texi2pdf_exe texi2pdf)
	if(texi2pdf_exe)
		add_custom_target(
			doc
			${texi2pdf_exe} ${CMAKE_CURRENT_SOURCE_DIR}/doc/clay.texi --output=${CMAKE_CURRENT_BINARY_DIR}/clay.pdf
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
			COMMENT "Generating documentation (pdf) (with texi2pdf)" VERBATIM
		)
	else()
		message (STATUS "texi2pdf not found :( Documentation can not be built")
	endif()


# osl
	find_package(osl REQUIRED)
	
# Clan
	find_package(clan REQUIRED)
	add_definitions(-DCLAN_LINKED)

# Candl
	find_package(candl REQUIRED)
	add_definitions(-DCANDL_LINKED)
	
# CLooG
	find_package(cloog-isl REQUIRED)
	add_definitions(-DCLOOG_LINKED)

# Flex
	find_package(BISON REQUIRED)
	find_package(FLEX REQUIRED)
	BISON_TARGET(clay_parser source/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.c)
	FLEX_TARGET(clay_scanner source/scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.c)
	ADD_FLEX_BISON_DEPENDENCY(clay_scanner clay_parser)
	include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Include directories (to use #include <> instead of #include "")

	# include/clay/macros.h
	configure_file("include/clay/macros.h.in" "include/clay/macros.h")
	include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")
	# clay
	include_directories("./include")


# Compiler log
	message(STATUS "---")
	message(STATUS "C compiler = ${CMAKE_C_COMPILER}")
	if (release)
		message(STATUS "Mode Release")
	else()
		message(STATUS "Mode Debug")
	endif()
	message(STATUS "C flags    = ${CMAKE_C_FLAGS}")


# Library

	message(STATUS "---")

	# files .c
	file(
		GLOB_RECURSE
		sources
		source/*
	)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/source/clay.c;" "" sources "${sources}") # with ;
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/source/clay.c" "" sources "${sources}")  # without ;

	# Shared
	add_library(
		clay
		SHARED
		${sources}
		${BISON_clay_parser_OUTPUTS}
		${FLEX_clay_scanner_OUTPUTS}
	)
	target_link_libraries(clay ${OSL_LIBRARY})
	target_link_libraries(clay ${CLAN_LIBRARY})
	target_link_libraries(clay ${CANDL_LIBRARY})
	target_link_libraries(clay ${CLOOG_ISL_LIBRARY})
	get_property(clay_lib_location TARGET clay PROPERTY LOCATION)
	message(STATUS "Add clay library (shared) ${clay_lib_location}")

	# Static
	add_library(
		clay_static
		STATIC
		${sources}
		${BISON_clay_parser_OUTPUTS}
		${FLEX_clay_scanner_OUTPUTS}
	)
	set_target_properties(clay_static PROPERTIES OUTPUT_NAME clay)
	target_link_libraries(clay_static ${OSL_LIBRARY})
	target_link_libraries(clay_static ${CLAN_LIBRARY})
	target_link_libraries(clay_static ${CANDL_LIBRARY})
	target_link_libraries(clay_static ${CLOOG_ISL_LIBRARY})
	get_property(clay_static_lib_location TARGET clay_static PROPERTY LOCATION)
	message(STATUS "Add clay library (static) ${clay_static_lib_location}")


# Executables & tests

	message(STATUS "---") # clay

	message(STATUS "Add executable clay")
	add_executable(clay_exe "source/clay.c")
	set_target_properties(clay_exe PROPERTIES OUTPUT_NAME "clay")
	target_link_libraries(clay_exe clay_static ${OSL_LIBRARY})
	target_link_libraries(clay_exe clay_static ${CLAN_LIBRARY})
	target_link_libraries(clay_exe clay_static ${CANDL_LIBRARY})
	target_link_libraries(clay_exe clay_static ${CLOOG_ISL_LIBRARY})

	# clay test
	find_package(PythonInterp)
	if (PYTHONINTERP_FOUND)

		message(STATUS "---")

		enable_testing()
		file(
			GLOB_RECURSE
			tests
			tests/*.c
		)

		foreach(test ${tests})
			message(STATUS "Add Test ${test}")
			add_test(
				${test}
				"${CMAKE_CURRENT_SOURCE_DIR}/tests/check_source_result.py"
				"${test}"
				"${test}.clay.scop"
				"${CMAKE_CURRENT_BINARY_DIR}/clay"
			)
			endforeach()

		message(STATUS "---")

	endif()


# Install

	install(TARGETS clay LIBRARY DESTINATION lib)
	install(TARGETS clay_static ARCHIVE DESTINATION lib)
	install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h")
	install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/" DESTINATION include FILES_MATCHING PATTERN "*.h")
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/parser.h" DESTINATION include/clay)
	install(FILES clay-config.cmake DESTINATION lib/clay)
	install(TARGETS clay_exe RUNTIME DESTINATION bin)
	install(DIRECTORY tests DESTINATION share/clay FILES_MATCHING PATTERN "*.c")


# Little help

	message(STATUS "You can execute:")
	message(STATUS "    make          # To compile clay library & clay")
	if (PYTHONINTERP_FOUND)
		message(STATUS "    make test     # To execute tests")
	endif()
	message(STATUS "    make install  # To install library, include and CMake module")
	message(STATUS "                  # If you need root access:")
	message(STATUS "                  #     sudo make install")
	message(STATUS "                  #     su -c \"make install\"")
	if(DOXYGEN_FOUND)
		message(STATUS "    make doxygen  # To generate the Doxygen")
	endif()
	if(texi2pdf_exe)
		message(STATUS "    make doc      # To generate the documentation")
	endif()

	message(STATUS "---")
