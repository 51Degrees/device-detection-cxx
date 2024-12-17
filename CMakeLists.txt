cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Include the common API

include(${CMAKE_CURRENT_LIST_DIR}/src/common-cxx/CMakeLists.txt NO_POLICY_SCOPE)

project(51DegreesDeviceDetection VERSION 4.0.1 LANGUAGES CXX C)

set(DD ${CMAKE_CURRENT_LIST_DIR}/src)
set(HASH ${CMAKE_CURRENT_LIST_DIR}/src/hash)

# Device Detection libraries
#Compute is test coverage enabled
string(TOLOWER "${CMAKE_BUILD_TYPE}" lower_CMAKE_BUILD_TYPE)
if (NOT MSVC AND "${lower_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	set(TESTCOVERAGE_ENABLED TRUE)
else()
	set(TESTCOVERAGE_ENABLED FALSE)
endif()
message("-- TESTCOVERAGE_ENABLED=${TESTCOVERAGE_ENABLED}")

FILE(GLOB DDC_SRC ${DD}/*.c)
FILE(GLOB DDC_H ${DD}/*.h)
add_library(fiftyone-device-detection-c ${DDC_SRC} ${DDC_H})
if (MSVC)
	target_link_libraries(fiftyone-device-detection-c fiftyone-common-c)
else()
	target_link_libraries(fiftyone-device-detection-c fiftyone-common-c m)
endif()

FILE(GLOB DDCPP_SRC ${DD}/*.cpp)
FILE(GLOB DDCPP_H ${DD}/*.hpp)
add_library(fiftyone-device-detection-cxx ${DDCPP_SRC} ${DDCPP_H})
target_link_libraries(fiftyone-device-detection-cxx
	fiftyone-device-detection-c
	fiftyone-common-cxx)

set_target_properties(fiftyone-device-detection-c fiftyone-device-detection-cxx
	PROPERTIES FOLDER "Device Detection") 

# Hash libraries

FILE(GLOB HASHC_SRC ${HASH}/*.c)
FILE(GLOB HASHC_H ${HASH}/*.h)

add_library(fiftyone-hash-c ${HASHC_SRC} ${HASHC_H})
target_link_libraries(fiftyone-hash-c fiftyone-device-detection-c)
if (MSVC)
	target_compile_options(fiftyone-hash-c PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
else()
	target_compile_options(fiftyone-hash-c PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
	if (MINGW)
		target_compile_options(fiftyone-hash-c PRIVATE "-Wa,-mbig-obj")
	endif()
endif()

FILE(GLOB HASHCPP_SRC ${HASH}/*.cpp)
FILE(GLOB HASHCPP_H ${HASH}/*.hpp)
add_library(fiftyone-hash-cxx ${HASHCPP_SRC} ${HASHCPP_H})
target_link_libraries(fiftyone-hash-cxx fiftyone-hash-c fiftyone-device-detection-cxx)
if (MSVC)
	target_compile_options(fiftyone-hash-cxx PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
else()
	target_compile_options(fiftyone-hash-cxx PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
	# Test coverage instrumentation
	if (TESTCOVERAGE_ENABLED)
		add_library(fiftyone-device-detection-c-cov ${DDC_SRC} ${DDC_H})
		target_link_libraries(fiftyone-device-detection-c-cov fiftyone-common-c m)
		target_compile_options(fiftyone-device-detection-c-cov PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror" "--coverage")

		add_library(fiftyone-device-detection-cxx-cov ${DDCPP_SRC} ${DDCPP_H})
		target_link_libraries(fiftyone-device-detection-cxx-cov fiftyone-device-detection-c-cov fiftyone-common-cxx)
		target_compile_options(fiftyone-device-detection-cxx-cov PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror" "--coverage")

		add_library(fiftyone-hash-c-cov ${HASHC_SRC} ${HASHC_H})
		target_link_libraries(fiftyone-hash-c-cov fiftyone-device-detection-c-cov)
		target_compile_options(fiftyone-hash-c-cov PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror" "--coverage")

		add_library(fiftyone-hash-cxx-cov ${HASHCPP_SRC} ${HASHCPP_H})
		target_link_libraries(fiftyone-hash-cxx-cov fiftyone-hash-c-cov fiftyone-device-detection-cxx-cov)
		target_compile_options(fiftyone-hash-cxx-cov PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror" "--coverage")
	endif()
endif()

set_target_properties(fiftyone-hash-c fiftyone-hash-cxx
	PROPERTIES FOLDER "Device Detection/Hash") 

# Shared library

file(WRITE null.cpp "")
add_library(fiftyone-device-detection-complete SHARED null.cpp)
target_link_libraries(fiftyone-device-detection-complete fiftyone-hash-cxx)
if (MSVC)
	target_compile_options(fiftyone-device-detection-complete PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
else()
	target_compile_options(fiftyone-device-detection-complete PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
endif()

set_target_properties(fiftyone-device-detection-complete PROPERTIES PUBLIC_HEADER "${HASH}/hash.h")

install(TARGETS fiftyone-device-detection-complete DESTINATION fiftyone-device-detection-complete
	RUNTIME DESTINATION lib
	ARCHIVE DESTINATION lib/static
	PUBLIC_HEADER DESTINATION include
	# Set this to optional, otherwise it will interfere with builds which
	# include this CMake file.
	OPTIONAL)

# Examples

MESSAGE("-- Looking for examples...")
foreach (api "Hash")
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
					${CMAKE_CURRENT_LIST_DIR}/examples/C/${api}/ExampleBase.c
					${CMAKE_CURRENT_LIST_DIR}/examples/C/${api}/ExampleBase.h
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/ExampleBase.cpp
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/ExampleBase.hpp)
					if (MSVC)
						target_link_libraries(${name} fiftyone-${lowerapi}-cxx)
					else()
						if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
							target_include_directories(${name} PRIVATE "/usr/local/include")
							target_link_directories(${name} PRIVATE "/usr/local/lib")
						endif()
						target_link_libraries(${name} fiftyone-${lowerapi}-cxx)
					endif()
				elseif("${langext}" STREQUAL "c")
					MESSAGE("-- Found C example '${name}'")
					add_executable(${name} ${file}
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/ExampleBase.c
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${api}/ExampleBase.h)
					if (MSVC)
						target_link_libraries(${name} fiftyone-${lowerapi}-c)
					else()
						if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
							target_include_directories(${name} PRIVATE "/usr/local/include")
							target_link_directories(${name} PRIVATE "/usr/local/lib")
						endif()
						target_link_libraries(${name} fiftyone-${lowerapi}-c)
					endif()
				endif()
				set_target_properties(${name}
					PROPERTIES FOLDER
					"Examples/Device Detection/${api}/${upperlangext}")
				if (MSVC)
					target_compile_options(${name} PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
					target_link_options(${name} PRIVATE "/WX")
				else()
					target_compile_options(${name} PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
				endif()
			endif()
		endforeach()
	endforeach()
endforeach()

# Tests

if(BUILD_TESTING)
	set(COMMON_TEST ${CMAKE_CURRENT_LIST_DIR}/src/common-cxx/tests)
	set(HASH_TEST ${CMAKE_CURRENT_LIST_DIR}/test/hash)
	set(DD_TEST	${CMAKE_CURRENT_LIST_DIR}/test)

	FILE(GLOB DD_TEST_SRC ${DD_TEST}/*.cpp)
	FILE(GLOB DD_TEST_H ${DD_TEST}/*.hpp)
	FILE(GLOB HASH_TEST_SRC ${HASH_TEST}/*.cpp)
	FILE(GLOB HASH_TEST_H ${HASH_TEST}/*.hpp)

	add_library(fiftyone-device-detection-test-base
		${DD_TEST_SRC} ${DD_TEST_H}
		${COMMON_TEST}/Base.cpp
		${COMMON_TEST}/EngineTests.cpp
		${COMMON_TEST}/ExampleTests.cpp)
	target_link_libraries(fiftyone-device-detection-test-base gtest_main)
	if (NOT MSVC)
		target_compile_options(fiftyone-device-detection-test-base PRIVATE ${COMPILE_OPTION_DEBUG})
	endif()

	set_target_properties(fiftyone-device-detection-test-base PROPERTIES FOLDER "Tests") 

	add_executable(HashTests 
		${HASH_TEST_SRC} ${HASH_TEST_H}
		${CMAKE_CURRENT_LIST_DIR}/examples/CPP/Hash/ExampleBase.cpp
		${CMAKE_CURRENT_LIST_DIR}/examples/CPP/Hash/ExampleBase.hpp)

	if (TESTCOVERAGE_ENABLED)
		target_link_libraries(HashTests fiftyone-device-detection-test-base fiftyone-hash-cxx-cov)
		target_link_options(HashTests PRIVATE "-fprofile-arcs" "-ftest-coverage")
	else()
		target_link_libraries(HashTests fiftyone-device-detection-test-base fiftyone-hash-cxx)
	endif()

	gtest_discover_tests(HashTests DISCOVERY_MODE PRE_TEST)

	set_target_properties(HashTests PROPERTIES FOLDER "Tests")

	if (MSVC)
		target_compile_options(HashTests PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
		target_link_options(HashTests PRIVATE "/WX")
	else ()
		target_compile_options(HashTests PRIVATE ${COMPILE_OPTION_DEBUG})
		target_link_options(HashTests PRIVATE "-fsanitize=address")
	endif()

	if (CMAKE_COMPILER_IS_GNUCC)
		target_compile_options(HashTests PRIVATE "-Wall" "-Werror" "-Wno-unused-variable" "-Wno-unused-result" "-Wno-unused-but-set-variable")
		if (MINGW)
			target_compile_options(HashTests PRIVATE "-Wa,-mbig-obj")
		endif()
	endif()
endif()
