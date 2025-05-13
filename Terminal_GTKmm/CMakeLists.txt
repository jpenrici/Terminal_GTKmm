cmake_minimum_required(VERSION 3.25)

project(TerminalApp LANGUAGES CXX)

set(PROGRAM_NAME "TerminalApp")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCES
    src/main.cpp
    src/interpreter.cpp
    src/terminal.cpp
)

add_executable(${PROGRAM_NAME} ${SOURCES})

find_package(PkgConfig REQUIRED)
if (PkgConfig_FOUND)
    pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
    include_directories(${GTKMM_INCLUDE_DIRS})
    target_include_directories(${PROGRAM_NAME} PRIVATE ${GTKMM_LIBRARY_DIRS})
    target_link_libraries(${PROGRAM_NAME} PRIVATE ${GTKMM_LIBRARIES})
    message(STATUS "PkgConfig found.")
    message(STATUS "GTKmm found:")
    message(STATUS "  Directory: ${GTKMM_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${GTKMM_LIBRARIES}")
else()
    message(FATAL_ERROR "PkgConfig or GTKmm not found. Please ensure the PkgConfig and GTKmm is installed on your system.")
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
if (Python3_FOUND)
    target_include_directories(${PROGRAM_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(${PROGRAM_NAME} PRIVATE ${Python3_LIBRARIES})   
    message(STATUS "Python3 found:")
    message(STATUS "  Directory: ${Python3_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${Python3_LIBRARIES}")
else()
    message(FATAL_ERROR "Python3 not found. Please ensure the Python3 library is installed on your system.")
endif()

find_package(Lua REQUIRED)
if (Lua_FOUND)
    target_include_directories(${PROGRAM_NAME} PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(${PROGRAM_NAME} PRIVATE ${LUA_LIBRARIES})
    message(STATUS "Lua found:")
    message(STATUS "  Directory: ${LUA_INCLUDE_DIR}")
    message(STATUS "  Libraries: ${LUA_LIBRARIES}")
else()
    message(FATAL_ERROR "Lua not found. Please ensure the Lua library is installed on your system.")
endif()

install(TARGETS ${PROGRAM_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR})
