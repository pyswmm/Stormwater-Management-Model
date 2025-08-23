#
# CMakeLists.txt - CMake configuration file for tests/shared
#
# Created: 9/17/2019
# Updated: 12/12/2019
#
# Author: Michael E. Tryby
#         US EPA - ORD/NRMRL
#

add_executable(test_cstrhelper
    test_cstrhelper.cpp
    $<TARGET_OBJECTS:shared_objs>
    )

target_include_directories(test_cstrhelper
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

target_link_libraries(
  test_cstrhelper
    PRIVATE
        boost_test_headers
)


add_executable(test_errormanager
    test_errormanager.cpp
    $<TARGET_OBJECTS:shared_objs>
    )

target_include_directories(test_errormanager
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

target_link_libraries(
  test_errormanager
    PRIVATE
        boost_test_headers
)


add_executable(test_filemanager
    test_filemanager.cpp
    $<TARGET_OBJECTS:shared_objs>
    )

target_include_directories(test_filemanager
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )

target_link_libraries(
  test_filemanager
    PRIVATE
        boost_test_headers
)


set_target_properties(test_cstrhelper test_errormanager test_filemanager
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
