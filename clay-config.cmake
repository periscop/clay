# Try to find the clay library

# CLAY_FOUND       - System has clay lib
# CLAY_INCLUDE_DIR - The clay include directory
# CLAY_LIBRARY     - Library needed to use clay


if (CLAY_INCLUDE_DIR AND CLAY_LIBRARY)
	# Already in cache, be silent
	set(CLAY_FIND_QUIETLY TRUE)
endif()

find_path(CLAY_INCLUDE_DIR NAMES clay/clay.h)
find_library(CLAY_LIBRARY NAMES clay)

if (CLAY_LIBRARY AND CLAY_INCLUDE_DIR)
	message(STATUS "Library clay found =) ${CLAY_LIBRARY}")
else()
	message(STATUS "Library clay not found =(")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CLAY DEFAULT_MSG CLAY_INCLUDE_DIR CLAY_LIBRARY)

mark_as_advanced(CLAY_INCLUDE_DIR CLAY_LIBRARY)
