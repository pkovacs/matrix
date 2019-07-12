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

CHECK_INCLUDE_FILES("assert.h" HAVE_ASSERT_H)
IF(NOT HAVE_ASSERT_H)
    message(FATAL_ERROR "assert.h not found")
ENDIF()
CHECK_INCLUDE_FILES("errno.h" HAVE_ERRNO_H)
IF(NOT HAVE_ERRNO_H)
    message(FATAL_ERROR "errno.h not found")
ENDIF()
CHECK_INCLUDE_FILES("getopt.h" HAVE_GETOPT_H)
IF(NOT HAVE_GETOPT_H)
    message(FATAL_ERROR "getopt.h not found")
ENDIF()
CHECK_INCLUDE_FILES("math.h" HAVE_MATH_H)
IF(NOT HAVE_MATH_H)
    message(FATAL_ERROR "math.h not found")
ENDIF()
CHECK_INCLUDE_FILES("stdio.h" HAVE_STDIO_H)
IF(NOT HAVE_STDIO_H)
    message(FATAL_ERROR "stdio.h not found")
ENDIF()
CHECK_INCLUDE_FILES("stdlib.h" HAVE_STDLIB_H)
IF(NOT HAVE_STDLIB_H)
    message(FATAL_ERROR "stdlib.h not found")
ENDIF()
CHECK_INCLUDE_FILES("string.h" HAVE_STRING_H)
IF(NOT HAVE_STRING_H)
    message(FATAL_ERROR "string.h not found")
ENDIF()

set(CMAKE_REQUIRED_INCLUDES ${MPI_C_INCLUDE_PATH})
CHECK_INCLUDE_FILES("mpi.h" HAVE_MPI_H)
IF(NOT HAVE_MPI_H)
    message(FATAL_ERROR "mpi.h not found")
ENDIF()
unset(CMAKE_REQUIRED_INCLUDES)
