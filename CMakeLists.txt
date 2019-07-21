#
# MIT License
#
# Copyright (c) 2019 Philip Kovacs
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

project(matrix C)
cmake_minimum_required(VERSION 3.8.0 FATAL_ERROR)
find_package(MPI REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# ------------
# Local checks
# ------------
include(FindHeaders)
include(FindFunctions)
include(FindSymbols)
include(FindLibraries)
include(TestCompiler)

# -------------------
# Set project version
# -------------------
set(MATRIX_VERSION_MAJOR 0)
set(MATRIX_VERSION_MINOR 1)
set(MATRIX_VERSION_PATCH 0)

# -----------------
# Set compile flags
# -----------------
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wformat -Wformat-security -Wunused-result -Wall -Wextra -Werror -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")

if(NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release")
endif(NOT CMAKE_BUILD_TYPE)

# --------------
# Build config.h
# --------------
configure_file(
    ${CMAKE_SOURCE_DIR}/config.h.in
    ${CMAKE_BINARY_DIR}/config.h
)
add_definitions(-DHAVE_CONFIG_H)
IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        add_definitions(-DDEBUG)
ENDIF()

# -------------------
# Include directories
# -------------------
include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
)

# ------------------
# Add subdirectories
# ------------------
add_subdirectory(cannon)
add_subdirectory(summa)

# -----------------
# Add CPack support
# -----------------
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${MATRIX_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MATRIX_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MATRIX_VERSION_PATCH})
string(CONCAT CPACK_PACKAGE_VERSION
    "${MATRIX_VERSION_MAJOR}."
    "${MATRIX_VERSION_MINOR}."
    "${MATRIX_VERSION_PATCH}")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION})
include(CPack)

# vi:set ts=2 sw=2 et ai:
