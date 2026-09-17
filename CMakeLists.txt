# https://libcheck.github.io/check/
cmake_minimum_required(VERSION 3.5...3.31)

project(Sample C)

# Custom cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Configure whether libraries will be static or shared linked
set(BUILD_SHARED_LIBS OFF)

# Determine minimum C
set(CMAKE_C_STANDARD 11)

# Enable debugging (only if not already set via -DCMAKE_BUILD_TYPE=...)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Enable Coverage Tests
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -O0")

# Show all warning messages
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

add_subdirectory(src)

enable_testing()

add_subdirectory(tests)

# Add build target to generate Coverage Test Report to all compiled object files
find_program(GCOV gcov)
find_program(LCOV lcov)
find_program(GENHTML genhtml)

if(NOT GCOV OR NOT LCOV OR NOT GENHTML)
  message(WARNING "gcov/lcov/genhtml not found — 'gcov' and 'lcov' coverage targets will be unavailable")
else()
  add_custom_target(gcov
    COMMAND mkdir -p Coverage
    COMMAND find ${CMAKE_BINARY_DIR} -type f -name "*.c.o" -exec ${GCOV} {} \;
    COMMENT "Generating gcov coverage reports"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
  add_custom_command(TARGET gcov POST_BUILD
    COMMAND echo "Coverage reports generated."
    COMMAND echo "You can find them in ${CMAKE_BINARY_DIR}/Coverage"
  )

  add_custom_target(lcov
    COMMAND mkdir -p Coverage
    COMMAND ${LCOV} --directory .. --capture --output-file Coverage/lcov.info
    COMMAND ${GENHTML} -o Coverage Coverage/lcov.info
    COMMENT "Generating lcov coverage reports"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
  add_custom_command(TARGET lcov POST_BUILD
    COMMAND echo "Coverage reports generated."
    COMMAND echo "You can find them in ${CMAKE_BINARY_DIR}/Coverage"
  )

  # Add 'Coverage' folder into clean target
  set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES Coverage)
endif()