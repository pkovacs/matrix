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

check_function_exists("exit" HAVE_EXIT)
if(NOT HAVE_EXIT)
    message(FATAL_ERROR "exit not found")
endif()

check_function_exists("fclose" HAVE_FCLOSE)
if(NOT HAVE_FCLOSE)
    message(FATAL_ERROR "fclose not found")
endif()

check_function_exists("fopen" HAVE_FOPEN)
if(NOT HAVE_FOPEN)
    message(FATAL_ERROR "fopen not found")
endif()

check_function_exists("fprintf" HAVE_FPRINTF)
if(NOT HAVE_FPRINTF)
    message(FATAL_ERROR "fprintf not found")
endif()

check_function_exists("fscanf" HAVE_FSCANF)
if(NOT HAVE_FSCANF)
    message(FATAL_ERROR "fscanf not found")
endif()

check_function_exists("getopt_long" HAVE_GETOPT_LONG)
if(NOT HAVE_GETOPT_LONG)
    message(FATAL_ERROR "getopt_long not found")
endif()

check_function_exists("memcpy" HAVE_MEMCPY)
if(NOT HAVE_MEMCPY)
    message(FATAL_ERROR "memcpy not found")
endif()

check_function_exists("printf" HAVE_PRINTF)
if(NOT HAVE_PRINTF)
    message(FATAL_ERROR "printf not found")
endif()

check_function_exists("strerror" HAVE_STRERROR)
if(NOT HAVE_STRERROR)
    message(FATAL_ERROR "strerror not found")
endif()

check_function_exists("strncpy" HAVE_STRNCPY)
if(NOT HAVE_STRNCPY)
    message(FATAL_ERROR "strncpy not found")
endif()

set(CMAKE_REQUIRED_LIBRARIES ${MPI_C_LIBRARIES})
check_function_exists("MPI_Init" HAVE_MPI_INIT)
if(NOT HAVE_MPI_INIT)
    message(FATAL_ERROR "MPI_Init not found")
endif()

check_function_exists("MPI_Finalize" HAVE_MPI_FINALIZE)
if(NOT HAVE_MPI_FINALIZE)
    message(FATAL_ERROR "MPI_Finalize not found")
endif()
unset(CMAKE_REQUIRED_LIBRARIES)
