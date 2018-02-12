project(PowerCore)
set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.9)

add_executable(powergenerator generator/PowerGenerator.cc)


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated.interpret1.h
    COMMAND powergenerator ${CMAKE_CURRENT_SOURCE_DIR}/src/powerpc.ppcdef > ${CMAKE_CURRENT_BINARY_DIR}/generated.interpret1.h
    DEPENDS powergenerator src/powerpc.ppcdef
)

add_library(PowerCore src/PowerCore.cc ${CMAKE_CURRENT_BINARY_DIR}/generated.interpret1.h)
target_include_directories(PowerCore PUBLIC include)
target_include_directories(PowerCore PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(testbed test/main.cc)
target_link_libraries(testbed PowerCore)

add_executable(test-powerpc test/test-powerpc.cpp)
target_compile_definitions(test-powerpc PRIVATE EMU_POWERCORE)
target_link_libraries(test-powerpc PowerCore)
