# Try to find the clay library

# CLAY_FOUND       - System has clay lib
# CLAY_INCLUDE_DIR - The clay include directory
# CLAY_LIBRARY     - Library needed to use clay
# CLAY_TESTS_DIR   - Share


if (CLAY_INCLUDE_DIR AND CLAY_LIBRARY AND CLAY_TESTS_DIR)
	# Already in cache, be silent
	set(CLAY_FIND_QUIETLY TRUE)
endif()

find_path(CLAY_INCLUDE_DIR NAMES clay/clay.h)
find_library(CLAY_LIBRARY NAMES clay)
find_path(CLAY_TESTS_DIR NAMES share/clay)
set(CLAY_TESTS_DIR "${CLAY_TESTS_DIR}/share/clay")

if (CLAY_LIBRARY AND CLAY_INCLUDE_DIR)
	message(STATUS "Library clay found =) ${CLAY_LIBRARY}")
else()
	message(STATUS "Library clay not found =(")
endif()
if (CLAY_TESTS_DIR)
	message(STATUS "Clay tests found =) ${CLAY_TESTS_DIR}")
else()
	message(STATUS "Clay tests not found =(")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CLAY DEFAULT_MSG CLAY_INCLUDE_DIR CLAY_LIBRARY CLAY_TESTS_DIR)

mark_as_advanced(CLAY_INCLUDE_DIR CLAY_LIBRARY CLAY_TESTS_DIR)
