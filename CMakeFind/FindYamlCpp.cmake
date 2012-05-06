# Look for a directory containing YAMLCPP.
#
# The following values are defined
# YAMLCPP_INCLUDE_DIR - where to find vector, etc.
# YAMLCPP_LIBRARIES   - link against these to use YAMLCPP

find_path(YAMLCPP_INCLUDE_DIR yaml-cpp)

find_library(YAMLCPP_LIBRARY_DEBUG
  NAMES libyaml-cppmdd
)

# if we only have debug libraries, use them.
# that is surely better than nothing.
find_library(YAMLCPP_LIBRARY_RELEASE
  NAMES libyaml-cppmd
)

if (YAMLCPP_INCLUDE_DIR)
  if (YAMLCPP_LIBRARIES)
    set(YAMLCPP_FOUND "YES")
  endif()
endif()

mark_as_advanced(
  YAMLCPP_INCLUDE_DIR
  YAMLCPP_LIBRARIES
)
