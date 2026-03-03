#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Arithmetics::Arithmetics" for configuration "Debug"
set_property(TARGET Arithmetics::Arithmetics APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Arithmetics::Arithmetics PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libArithmetics.a"
  )

list(APPEND _cmake_import_check_targets Arithmetics::Arithmetics )
list(APPEND _cmake_import_check_files_for_Arithmetics::Arithmetics "${_IMPORT_PREFIX}/lib/libArithmetics.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
