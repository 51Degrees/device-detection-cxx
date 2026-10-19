# CMake configuration for js-snippet-export tool
cmake_minimum_required(VERSION 3.24)

# Add the executable
add_executable(js-snippet-export main.cpp)

# Link against the Hash engine library
target_link_libraries(js-snippet-export fiftyone-hash-cxx)

# Set compiler options
if (MSVC)
    target_compile_options(js-snippet-export PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
else()
    target_compile_options(js-snippet-export PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
endif()

# Set output directory
set_target_properties(js-snippet-export 
    PROPERTIES 
    FOLDER "Tools"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install target
install(TARGETS js-snippet-export 
    RUNTIME DESTINATION bin
)
