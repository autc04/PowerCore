project(PowerCore)
set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.9)

add_executable(ppcgen ParseInstructions.cc)


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated.interpret1.h
    COMMAND ppcgen ${CMAKE_CURRENT_SOURCE_DIR}/powerpc.ppcdef > ${CMAKE_CURRENT_BINARY_DIR}/generated.interpret1.h
    DEPENDS ppcgen powerpc.ppcdef
)

add_executable(ppctest main.cc ${CMAKE_CURRENT_BINARY_DIR}/generated.interpret1.h)
target_include_directories(ppctest PRIVATE ${CMAKE_CURRENT_BINARY_DIR})