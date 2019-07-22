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

include(CheckIncludeFiles)

check_include_files("assert.h" HAVE_ASSERT_H)
if(NOT HAVE_ASSERT_H)
    message(FATAL_ERROR "assert.h not found")
endif()

check_include_files("errno.h" HAVE_ERRNO_H)
if(NOT HAVE_ERRNO_H)
    message(FATAL_ERROR "errno.h not found")
endif()

check_include_files("getopt.h" HAVE_GETOPT_H)
if(NOT HAVE_GETOPT_H)
    message(FATAL_ERROR "getopt.h not found")
endif()

check_include_files("math.h" HAVE_MATH_H)
if(NOT HAVE_MATH_H)
    message(FATAL_ERROR "math.h not found")
endif()

check_include_files("stdio.h" HAVE_STDIO_H)
if(NOT HAVE_STDIO_H)
    message(FATAL_ERROR "stdio.h not found")
endif()

check_include_files("stdlib.h" HAVE_STDLIB_H)
if(NOT HAVE_STDLIB_H)
    message(FATAL_ERROR "stdlib.h not found")
endif()

check_include_files("string.h" HAVE_STRING_H)
if(NOT HAVE_STRING_H)
    message(FATAL_ERROR "string.h not found")
endif()

set(CMAKE_REQUIRED_INCLUDES ${MPI_C_INCLUDE_PATH})
check_include_files("mpi.h" HAVE_MPI_H)
if(NOT HAVE_MPI_H)
    message(FATAL_ERROR "mpi.h not found")
endif()
unset(CMAKE_REQUIRED_INCLUDES)
