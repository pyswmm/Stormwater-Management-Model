
cmake_minimum_required (VERSION 3.0)

#Setup CMake to run tests
enable_testing()

# Sets for output directory for executables and libraries. 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif(UNIX)

#Prep ourselves for compiling boost
find_package(Boost REQUIRED)
include_directories (${Boost_INCLUDE_DIRS})

# Test files are a separate source directory called tests
file(GLOB TEST_SRCS . tests/test_*.cpp)

# Set up file groups for exe and lib targets
file(GLOB COUPLING_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} . src/*.c)
# list(REMOVE_ITEM SWMM_SOURCES "src/coupling.c")
set(COUPLING_HEADERS include/coupling.h)
include_directories(include)

# Set compiler flags when building on UNIX.
if(UNIX AND NOT APPLE)
	set(CMAKE_C_FLAGS "-fopenmp -Ofast")
	set(CMAKE_CXX_FLAGS "-std=c++11 -fopenmp -Wno-write-strings -Ofast")
    # add_definitions(-std=c99)
endif(UNIX AND NOT APPLE)

# Loads settings for OpenMP and append any OpenMP compiler flags.
FIND_PACKAGE(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif(OPENMP_FOUND)

option(BUILD_TESTS "Build unit tests (requires Boost test)" OFF)
option(BUILD_COVERAGE "Build library for coverage" OFF)

# Build target for coverage configuration
IF (BUILD_COVERAGE)

include(CodeCoverage)
set(CMAKE_BUILD_TYPE "Debug")
APPEND_COVERAGE_COMPILER_FLAGS()
set(CMAKE_C_FLAGS_DEBUG "-O0")

add_library(coupling SHARED . src/coupling.c)
target_include_directories(coupling PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Build target for default library configuration
ELSE (BUILD_COVERAGE)

add_library(coupling SHARED . src/coupling.c . src/error.c)
target_include_directories(coupling PUBLIC ${PROJECT_SOURCE_DIR}/include)

ENDIF (BUILD_COVERAGE)

#Run through each source
foreach(testSrc ${TEST_SRCS})
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc})

        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName} ${Boost_LIBRARIES} coupling)

        #Finally add it to test execution - 
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName}
                 COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testName})  
endforeach(testSrc)


