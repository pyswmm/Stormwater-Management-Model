#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "swmm5" for configuration "Release"
set_property(TARGET swmm5 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(swmm5 PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libswmm5.so"
  IMPORTED_SONAME_RELEASE "libswmm5.so"
  )

list(APPEND _cmake_import_check_targets swmm5 )
list(APPEND _cmake_import_check_files_for_swmm5 "${_IMPORT_PREFIX}/lib/libswmm5.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
