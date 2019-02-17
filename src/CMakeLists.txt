#-----------------------------------------------------------------------------
#
#  CMake Config
#
#-----------------------------------------------------------------------------

add_executable(ope main.cpp)
target_link_libraries(ope ${OSMIUM_IO_LIBRARIES})
install(TARGETS ope DESTINATION bin)

