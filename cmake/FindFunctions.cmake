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

include(CheckFunctionExists)

CHECK_FUNCTION_EXISTS("exit" HAVE_EXIT)
IF(NOT HAVE_EXIT)
    message(FATAL_ERROR "exit not found")
ENDIF()
CHECK_FUNCTION_EXISTS("fclose" HAVE_FCLOSE)
IF(NOT HAVE_FCLOSE)
    message(FATAL_ERROR "fclose not found")
ENDIF()
CHECK_FUNCTION_EXISTS("fopen" HAVE_FOPEN)
IF(NOT HAVE_FOPEN)
    message(FATAL_ERROR "fopen not found")
ENDIF()
CHECK_FUNCTION_EXISTS("fprintf" HAVE_FPRINTF)
IF(NOT HAVE_FPRINTF)
    message(FATAL_ERROR "fprintf not found")
ENDIF()
CHECK_FUNCTION_EXISTS("fscanf" HAVE_FSCANF)
IF(NOT HAVE_FSCANF)
    message(FATAL_ERROR "fscanf not found")
ENDIF()
CHECK_FUNCTION_EXISTS("getopt_long" HAVE_GETOPT_LONG)
IF(NOT HAVE_GETOPT_LONG)
    message(FATAL_ERROR "getopt_long not found")
ENDIF()
CHECK_FUNCTION_EXISTS("memcpy" HAVE_MEMCPY)
IF(NOT HAVE_MEMCPY)
    message(FATAL_ERROR "memcpy not found")
ENDIF()
CHECK_FUNCTION_EXISTS("printf" HAVE_PRINTF)
IF(NOT HAVE_PRINTF)
    message(FATAL_ERROR "printf not found")
ENDIF()
CHECK_FUNCTION_EXISTS("strerror" HAVE_STRERROR)
IF(NOT HAVE_STRERROR)
    message(FATAL_ERROR "strerror not found")
ENDIF()
CHECK_FUNCTION_EXISTS("strncpy" HAVE_STRNCPY)
IF(NOT HAVE_STRNCPY)
    message(FATAL_ERROR "strncpy not found")
ENDIF()

set(CMAKE_REQUIRED_LIBRARIES ${MPI_C_LIBRARIES})
CHECK_FUNCTION_EXISTS("MPI_Init" HAVE_MPI_INIT)
IF(NOT HAVE_MPI_INIT)
    message(FATAL_ERROR "MPI_Init not found")
ENDIF()
CHECK_FUNCTION_EXISTS("MPI_Finalize" HAVE_MPI_FINALIZE)
IF(NOT HAVE_MPI_FINALIZE)
    message(FATAL_ERROR "MPI_Finalize not found")
ENDIF()
unset(CMAKE_REQUIRED_LIBRARIES)
