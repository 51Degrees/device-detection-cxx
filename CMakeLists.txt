cmake_minimum_required(VERSION 3.5)
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 11)

# Include the common API

include(${CMAKE_CURRENT_LIST_DIR}/src/common-cxx/CMakeLists.txt NO_POLICY_SCOPE)

project(51DegreesDeviceDetection VERSION 4.0.1 LANGUAGES CXX C)

set(DD ${CMAKE_CURRENT_LIST_DIR}/src)
set(HASH ${CMAKE_CURRENT_LIST_DIR}/src/hash)
set(PATTERN ${CMAKE_CURRENT_LIST_DIR}/src/pattern)

# Device Detection libraries

FILE(GLOB DDC_SRC ${DD}/*.c)
FILE(GLOB DDC_H ${DD}/*.h)
add_library(fiftyone-device-detection-c ${DDC_SRC} ${DDC_H})
target_link_libraries(fiftyone-device-detection-c fiftyone-common-c)

FILE(GLOB DDCPP_SRC ${DD}/*.cpp)
FILE(GLOB DDCPP_H ${DD}/*.hpp)
add_library(fiftyone-device-detection-cxx ${DDCPP_SRC} ${DDCPP_H})
target_link_libraries(fiftyone-device-detection-cxx
	fiftyone-device-detection-c
	fiftyone-common-cxx)

set_target_properties(fiftyone-device-detection-c fiftyone-device-detection-cxx
	PROPERTIES FOLDER "Device Detection") 

# Hash libraries

add_library(fiftyone-hash-c ${HASH}/hash.c ${HASH}/hash.h)
target_link_libraries(fiftyone-hash-c fiftyone-device-detection-c)
if (MSVC)
	target_compile_options(fiftyone-hash-c PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
endif()

FILE(GLOB HASHCPP_SRC ${HASH}/*.cpp)
FILE(GLOB HASHCPP_H ${HASH}/*.hpp)
add_library(fiftyone-hash-cxx ${HASHCPP_SRC} ${HASHCPP_H})
target_link_libraries(fiftyone-hash-cxx
	fiftyone-hash-c
	fiftyone-device-detection-cxx)

set_target_properties(fiftyone-hash-c fiftyone-hash-cxx
	PROPERTIES FOLDER "Device Detection/Hash") 

# Pattern libraries

FILE(GLOB PATTERNC_SRC ${PATTERN}/*.c)
FILE(GLOB PATTERNC_H ${PATTERN}/*.h)
add_library(fiftyone-pattern-c ${PATTERNC_SRC} ${PATTERNC_H}
	${DD}/cityhash/city.c ${DD}/cityhash/city.h)
target_link_libraries(fiftyone-pattern-c fiftyone-device-detection-c)
if (MSVC)
	target_compile_options(fiftyone-pattern-c PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
endif()

FILE(GLOB PATTERNCPP_SRC ${PATTERN}/*.cpp)
FILE(GLOB PATTERNCPP_H ${PATTERN}/*.hpp)
add_library(fiftyone-pattern-cxx ${PATTERNCPP_SRC} ${PATTERNCPP_H})
target_link_libraries(fiftyone-pattern-cxx
	fiftyone-pattern-c
	fiftyone-device-detection-cxx)

set_target_properties(fiftyone-pattern-c fiftyone-pattern-cxx
	PROPERTIES FOLDER "Device Detection/Pattern") 

# Shared library

file(WRITE null.cpp "")
add_library(fiftyone-device-detection-complete SHARED null.cpp)
target_link_libraries(fiftyone-device-detection-complete fiftyone-pattern-cxx fiftyone-hash-cxx)

set_target_properties(fiftyone-device-detection-complete PROPERTIES PUBLIC_HEADER "${PATTERN}/pattern.h;${HASH}/hash.h")

install(TARGETS fiftyone-device-detection-complete DESTINATION fiftyone-device-detection-complete
	RUNTIME DESTINATION lib
	ARCHIVE DESTINATION lib/static
	PUBLIC_HEADER DESTINATION include)

# Examples

MESSAGE("-- Looking for examples...")
foreach (api "Hash" "Pattern")
	foreach (langext c cpp)
		string( TOUPPER ${langext} upperlangext)
		file(GLOB files ${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/*.${langext})
		foreach(file ${files})
			string( REPLACE ".${langext}" "" name ${file})
			string( REPLACE "${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/" "" name ${name})
			string( APPEND name ${api} ${upperlangext})
			string( TOLOWER ${api} lowerapi)

			if (NOT ${name} MATCHES ".*ExampleBase.*")
				if("${langext}" STREQUAL "cpp")
					MESSAGE("-- Found C++ example '${name}'")
					add_executable(${name} ${file}
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/ExampleBase.cpp
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/ExampleBase.hpp)
					target_link_libraries(${name} fiftyone-${lowerapi}-cxx)
				elseif("${langext}" STREQUAL "c")
					MESSAGE("-- Found C example '${name}'")
					add_executable(${name} ${file})
					target_link_libraries(${name} fiftyone-${lowerapi}-c)
				endif()
				set_target_properties(${name}
					PROPERTIES FOLDER
					"Examples/Device Detection/${api}/${upperlangext}") 
				if (MSVC)
					target_compile_options(${name} PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
				endif()
			endif()
		endforeach()
	endforeach()
endforeach()

# Tests

set(COMMON_TEST ${CMAKE_CURRENT_LIST_DIR}/src/common-cxx/tests)
set(PATTERN_TEST ${CMAKE_CURRENT_LIST_DIR}/test/pattern)
set(HASH_TEST ${CMAKE_CURRENT_LIST_DIR}/test/hash)
set(DD_TEST	${CMAKE_CURRENT_LIST_DIR}/test)

FILE(GLOB DD_TEST_SRC ${DD_TEST}/*.cpp)
FILE(GLOB DD_TEST_H ${DD_TEST}/*.hpp)
FILE(GLOB PATTERN_TEST_SRC ${PATTERN_TEST}/*.cpp)
FILE(GLOB PATTERN_TEST_H ${PATTERN_TEST}/*.hpp)
FILE(GLOB HASH_TEST_SRC ${HASH_TEST}/*.cpp)
FILE(GLOB HASH_TEST_H ${HASH_TEST}/*.hpp)

add_library(fiftyone-device-detection-test-base
	${DD_TEST_SRC} ${DD_TEST_H}
	${COMMON_TEST}/Base.cpp
	${COMMON_TEST}/EngineTests.cpp
	${COMMON_TEST}/ExampleTests.cpp)
target_link_libraries(fiftyone-device-detection-test-base gtest_main)

set_target_properties(fiftyone-device-detection-test-base PROPERTIES FOLDER "Tests") 

add_executable(PatternTests	${PATTERN_TEST_SRC} ${PATTERN_TEST_H})
target_link_libraries(PatternTests
	fiftyone-device-detection-test-base
	fiftyone-pattern-cxx)
gtest_discover_tests(PatternTests)

add_executable(HashTests ${HASH_TEST_SRC} ${HASH_TEST_H})
target_link_libraries(HashTests
	fiftyone-device-detection-test-base
    fiftyone-hash-cxx)
gtest_discover_tests(HashTests)

set_target_properties(HashTests PatternTests PROPERTIES FOLDER "Tests")

if (MSVC)
	target_compile_options(HashTests PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
	target_compile_options(PatternTests PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
	target_compile_options(PatternTests PRIVATE "-Wall" "-Wno-unused-variable" "-Wno-unused-result")
	target_compile_options(HashTests PRIVATE "-Wall" "-Wno-unused-variable" "-Wno-unused-result" "-Wno-unused-but-set-variable")
endif()
