#-----------------------------------------------------------------------------
#
#  CMake Config
#
#  OSM-PostrgreSQL-Experiments - Tests
#
#-----------------------------------------------------------------------------

include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/vendor/catch2/include)

include_directories(${CMAKE_SOURCE_DIR}/src)

set(UNIT_TESTS test-util.cpp)

add_executable(unit_tests unit_tests.cpp ${UNIT_TESTS} ../src/util.cpp)
add_test(NAME unit_tests COMMAND unit_tests WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

#-----------------------------------------------------------------------------
