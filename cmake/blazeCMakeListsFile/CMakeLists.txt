#==================================================================================================
#
#   CMake file exposing Blaze configuration and allowing integration in CMake projects.
#
#   Copyright (C) 2012-2020 Klaus Iglberger - All Rights Reserved
#
#   This file is part of the Blaze library. You can redistribute it and/or modify it under
#   the terms of the New (Revised) BSD License. Redistribution and use in source and binary
#   forms, with or without modification, are permitted provided that the following conditions
#   are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this list of
#      conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright notice, this list
#      of conditions and the following disclaimer in the documentation and/or other materials
#      provided with the distribution.
#   3. Neither the names of the Blaze development group nor the names of its contributors
#      may be used to endorse or promote products derived from this software without specific
#      prior written permission.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
#   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
#   SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
#   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
#   DAMAGE.
#
#==================================================================================================

cmake_minimum_required(VERSION 3.5)

file(READ ${CMAKE_CURRENT_LIST_DIR}/blaze/system/Version.h BLAZE_VERSION_FILE)

string(REGEX MATCH "#define BLAZE_MAJOR_VERSION ([0-9]*)" _BLAZE_MAJOR_VERSION ${BLAZE_VERSION_FILE})
set(BLAZE_MAJOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "#define BLAZE_MINOR_VERSION ([0-9]*)" _BLAZE_MINOR_VERSION ${BLAZE_VERSION_FILE})
set(BLAZE_MINOR_VERSION ${CMAKE_MATCH_1})

project(blaze LANGUAGES CXX VERSION "${BLAZE_MAJOR_VERSION}.${BLAZE_MINOR_VERSION}")

message(STATUS "Configuring blaze version ${PROJECT_VERSION}")

add_library(blaze INTERFACE)
target_include_directories(blaze INTERFACE
   $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
   message(STATUS "Blaze requires compiling in mode supporting C++14 features.")
else()
   target_compile_features(blaze INTERFACE cxx_std_14)
endif()


#==================================================================================================
# Configure LAPACK
#==================================================================================================

# Blaze has a link time dependency on several LAPACK operations (matrix decomposition, matrix
# inversion, eigenvalue computation, ...). However, this check here in combination with making
# the LAPACK package REQUIRED makes sure that packagers don't accidently forget this dependency
# on LAPACK.
option (USE_LAPACK "If enabled, checks for the presence of LAPACK during configuration. Disable if you plan on not using LAPACK operations." ON)

if (USE_LAPACK)
   target_link_libraries(blaze INTERFACE OpenBLAS::OpenBLAS)
endif()


#==================================================================================================
# Configure cache size
#==================================================================================================

set(BLAZE_CACHE_SIZE_DEFAULT "3072" CACHE INTERNAL "Default value for the cache size in Kilobytes.")
set(BLAZE_CACHE_SIZE_AUTO ON CACHE BOOL "Find automatically the cache size.")

if (${BLAZE_CACHE_SIZE_AUTO})
   set(flag 1)
   if (WIN32)
      execute_process(COMMAND wmic cpu get L3CacheSize
                      OUTPUT_VARIABLE tmp
                      RESULT_VARIABLE flag
                      ERROR_QUIET)
      if (flag OR "${tmp}" STREQUAL "")
         execute_process(COMMAND wmic cpu get L2CacheSize
                         OUTPUT_VARIABLE tmp
                         RESULT_VARIABLE flag
                         ERROR_QUIET)
      endif (flag OR "${tmp}" STREQUAL "")
      if (flag OR "${tmp}" STREQUAL "")
         execute_process(COMMAND wmic cpu get L1CacheSize
                         OUTPUT_VARIABLE tmp
                         RESULT_VARIABLE flag
                         ERROR_QUIET)
      endif (flag OR "${tmp}" STREQUAL "")
   endif (WIN32)

   if (UNIX)
      execute_process(COMMAND cat /sys/devices/system/cpu/cpu0/cache/index3/size
                      OUTPUT_VARIABLE tmp
                      RESULT_VARIABLE flag
                      ERROR_QUIET)
      if (flag OR "${tmp}" STREQUAL "")
         execute_process(COMMAND cat /sys/devices/system/cpu/cpu0/cache/index2/size
                         OUTPUT_VARIABLE tmp
                         RESULT_VARIABLE flag
                         ERROR_QUIET)
      endif (flag OR "${tmp}" STREQUAL "")
      if (flag OR "${tmp}" STREQUAL "")
         execute_process(COMMAND cat /sys/devices/system/cpu/cpu0/cache/index1/size
                         OUTPUT_VARIABLE tmp
                         RESULT_VARIABLE flag
                         ERROR_QUIET)
      endif (flag OR "${tmp}" STREQUAL "")
   endif (UNIX)

   if (APPLE)
      execute_process(COMMAND sysctl -n hw.l3cachesize
                      OUTPUT_VARIABLE tmp
                      RESULT_VARIABLE flag
                      ERROR_QUIET)
      if (flag OR "${tmp}" STREQUAL "")
         execute_process(COMMAND sysctl -n hw.l2cachesize
                         OUTPUT_VARIABLE tmp
                         RESULT_VARIABLE flag
                         ERROR_QUIET)
      endif (flag OR "${tmp}" STREQUAL "")
      if (flag OR "${tmp}" STREQUAL "")
         execute_process(COMMAND sysctl -n hw.l1icachesize
                         OUTPUT_VARIABLE tmp
                         RESULT_VARIABLE flag
                         ERROR_QUIET)
      endif (flag OR "${tmp}" STREQUAL "")

      if (flag EQUAL 0 AND NOT "${tmp}" STREQUAL "")
         math(EXPR tmp "${tmp}/1024")  # If successful convert to kibibytes to comply with rest
      endif (flag EQUAL 0 AND NOT "${tmp}" STREQUAL "")
   endif (APPLE)

   string(REGEX MATCH "([0-9][0-9]+)" tmp "${tmp}") # Get a number containing at least 2 digits in the string tmp

   if (flag OR NOT tmp)
      message("Cache size not found automatically. Using default value as cache size.")
      set(tmp ${BLAZE_CACHE_SIZE_DEFAULT})
   endif (flag OR NOT tmp)

   math(EXPR BLAZE_CACHE_SIZE "${tmp}*1024") # Convert to bytes (assuming that the value is given in kibibytes)

endif (${BLAZE_CACHE_SIZE_AUTO})

if (NOT ${BLAZE_CACHE_SIZE_AUTO})
   message("Use value set manually as cache size.")
   set(BLAZE_CACHE_SIZE ${BLAZE_CACHE_SIZE})
endif ()

set(BLAZE_CACHE_SIZE ${BLAZE_CACHE_SIZE} CACHE STRING "Cache size of the CPU in bytes." FORCE)

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/CacheSize.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/CacheSize.h")


#==================================================================================================
# Configure BLAS
#==================================================================================================

set(BLAZE_BLAS_MODE OFF CACHE BOOL "Enables/Disable the BLAS mode.")

if (BLAZE_BLAS_MODE)
   set(BLAZE_BLAS_IS_64BIT 0 CACHE BOOL "Specify whether the used BLAS library itself is 64-bit or not.")
   set(BLAZE_BLAS_IS_PARALLEL 0 CACHE BOOL "Specify whether the used BLAS library is itself parallelized or not.")
   set(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION 0 CACHE BOOL "Use BLAS kernels for dense matrix/vector multiplications instead of the default Blaze kernels.")
   set(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION 1 CACHE BOOL "Use BLAS kernels for dense matrix multiplications instead of the default Blaze kernels.")
   set(BLAZE_BLAS_INCLUDE_FILE "<cblas.h>" CACHE STRING "Specify the name of the BLAS header file (e.g. with MKL the file is called mkl_cblas.h)")
else ()
   unset(BLAZE_BLAS_IS_64BIT CACHE)
   unset(BLAZE_BLAS_IS_PARALLEL CACHE)
   unset(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION CACHE)
   unset(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION CACHE)
   unset(BLAZE_BLAS_INCLUDE_FILE CACHE)
endif ()

if (BLAZE_BLAS_MODE)
   set(BLAZE_BLAS_MODE 1)
else ()
   set(BLAZE_BLAS_MODE 0)
endif ()

if (BLAZE_BLAS_IS_64BIT)
   set(BLAZE_BLAS_IS_64BIT 1)
else ()
   set(BLAZE_BLAS_IS_64BIT 0)
endif ()

if (BLAZE_BLAS_IS_PARALLEL)
   set(BLAZE_BLAS_IS_PARALLEL 1)
else ()
   set(BLAZE_BLAS_IS_PARALLEL 0)
endif ()

if (BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION)
   set(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION 1)
else ()
   set(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION 0)
endif ()

if (BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION)
   set(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION 1)
else ()
   set(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION 0)
endif ()

if (BLAZE_BLAS_INCLUDE_FILE)
   set(BLAZE_BLAS_INCLUDE_FILE ${BLAZE_BLAS_INCLUDE_FILE})
else ()
   set(BLAZE_BLAS_INCLUDE_FILE "<cblas.h>")
endif ()

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/BLAS.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/BLAS.h")


#==================================================================================================
# Configure transpose flag
#==================================================================================================

set(BLAZE_TRANSPOSE_FLAG "blaze::columnVector" CACHE STRING "Specify the default transpose flag for all vectors of the Blaze library.")
set_property(CACHE BLAZE_TRANSPOSE_FLAG PROPERTY STRINGS "blaze::columnVector;blaze::rowVector")

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/TransposeFlag.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/TransposeFlag.h")


#==================================================================================================
# Configure storage order
#==================================================================================================

set(BLAZE_STORAGE_ORDER "blaze::rowMajor" CACHE STRING "Specify the default storage order for all matrices of the Blaze library.")
set_property(CACHE BLAZE_STORAGE_ORDER PROPERTY STRINGS "blaze::rowMajor;blaze::columnMajor")

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/StorageOrder.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/StorageOrder.h")


#==================================================================================================
# Configure alignment
#==================================================================================================

set(BLAZE_ALIGNMENT "blaze::aligned" CACHE STRING "Specify the default alignment for all vectors and matrices of the Blaze library.")
set_property(CACHE BLAZE_ALIGNMENT PROPERTY STRINGS "blaze::aligned;blaze::unaligned")

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/Alignment.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/Alignment.h")


#==================================================================================================
# Configure padding
#==================================================================================================

set(BLAZE_PADDING "blaze::padded" CACHE STRING "Specify the default padding flag for all vectors and matrices of the Blaze library")
set_property(CACHE BLAZE_PADDING PROPERTY STRINGS "blaze::padded;blaze::unpadded")

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/Padding.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/Padding.h")


#==================================================================================================
# Configure vectorization
#==================================================================================================

set(BLAZE_VECTORIZATION ON CACHE BOOL "Enable/Disable the vectorization of mathematical expressions via the SSE, AVX, and/or MIC instruction sets.")

if (BLAZE_VECTORIZATION)
   set(BLAZE_VECTORIZATION 1)
else ()
   set(BLAZE_VECTORIZATION 0)
endif ()

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/Vectorization.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/Vectorization.h")


#==================================================================================================
# Configure shared memory parallelization
#==================================================================================================

set(BLAZE_SHARED_MEMORY_PARALLELIZATION ON CACHE BOOL "Enable/Disable the shared-memory parallelization.")

if (BLAZE_SHARED_MEMORY_PARALLELIZATION)
   set(BLAZE_SHARED_MEMORY_PARALLELIZATION 1)
   set(BLAZE_SMP_THREADS "OpenMP" CACHE STRING "Specify which thread library is used for shared-memory parallelization.")
   set_property(CACHE BLAZE_SMP_THREADS PROPERTY STRINGS "OpenMP;C++11;Boost;HPX")
else ()
   set(BLAZE_SHARED_MEMORY_PARALLELIZATION 0)
   unset(BLAZE_SMP_THREADS CACHE)
endif ()

if (BLAZE_SMP_THREADS STREQUAL "OpenMP")
   find_package(OpenMP)
   if (OPENMP_FOUND)
      target_compile_options(blaze INTERFACE ${OpenMP_CXX_FLAGS})
      if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
         target_link_libraries(blaze INTERFACE ${OpenMP_CXX_FLAGS}) # Needed for GNU and Clang
      endif ()
   else ()
      message(WARNING "OpenMP not found. Blaze is running on a single thread. Try C++11 or Boost.")
   endif ()
elseif (BLAZE_SMP_THREADS STREQUAL "C++11")
   find_package(Threads REQUIRED)
   target_compile_definitions(blaze INTERFACE BLAZE_USE_CPP_THREADS)
   target_link_libraries(blaze INTERFACE ${CMAKE_THREAD_LIBS_INIT})
elseif (BLAZE_SMP_THREADS STREQUAL "Boost")
   find_package(Boost REQUIRED COMPONENTS thread)
   target_compile_definitions(blaze INTERFACE BLAZE_USE_BOOST_THREADS)
   target_include_directories(blaze INTERFACE ${Boost_INCLUDE_DIRS})
   target_compile_definitions(blaze INTERFACE BOOST_ALL_DYN_LINK ) # Needed for Visual Studio 2015
   target_link_libraries(blaze INTERFACE ${Boost_LIBRARIES})
elseif (BLAZE_SMP_THREADS STREQUAL "HPX")
   find_package(HPX REQUIRED)
   target_compile_definitions(blaze INTERFACE BLAZE_USE_HPX_THREADS)
   target_include_directories(blaze INTERFACE ${HPX_INCLUDE_DIRS})
   target_link_libraries(blaze INTERFACE ${HPX_LIBRARIES})
endif ()

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/SMP.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/SMP.h")


#==================================================================================================
# Configure shared memory parallelization
#==================================================================================================

set(BLAZE_RESTRICT ON CACHE BOOL "Enable/Disable the C99 restrict keyword.")

if (BLAZE_RESTRICT)
   set(BLAZE_RESTRICT 1)
else ()
   set(BLAZE_RESTRICT 0)
endif ()

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/Restrict.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/Restrict.h")


#==================================================================================================
# Configure optimizations
#==================================================================================================

set(BLAZE_OPTIMIZATION_PADDING ON CACHE BOOL "Enable/Disable the padding of dense vectors and matrices.")
set(BLAZE_OPTIMIZATION_STREAMING ON CACHE BOOL "Enable/Disable streaming (i.e. non-temporal stores).")
set(BLAZE_OPTIMIZATION_KERNELS ON CACHE BOOL "Enable/Disable all optimized compute kernels of the Blaze library.")
set(BLAZE_OPTIMIZATION_INITIALIZATION ON CACHE BOOL "Enable/Disable the default initialization of StaticVector and StaticMatrix.")

if (BLAZE_OPTIMIZATION_PADDING)
   set(BLAZE_OPTIMIZATION_PADDING "1")
else ()
   set(BLAZE_OPTIMIZATION_PADDING "0")
endif ()

if (BLAZE_OPTIMIZATION_STREAMING)
   set(BLAZE_OPTIMIZATION_STREAMING "1")
else ()
   set(BLAZE_OPTIMIZATION_STREAMING "0")
endif ()

if (BLAZE_OPTIMIZATION_KERNELS)
   set(BLAZE_OPTIMIZATION_KERNELS "1")
else ()
   set(BLAZE_OPTIMIZATION_KERNELS "0")
endif ()

if (BLAZE_OPTIMIZATION_INITIALIZATION)
   set(BLAZE_OPTIMIZATION_INITIALIZATION "1")
else ()
   set(BLAZE_OPTIMIZATION_INITIALIZATION "0")
endif ()

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/Optimizations.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/Optimizations.h")


#==================================================================================================
# Configure inline
#==================================================================================================

set(BLAZE_INLINE_ALWAYS ON CACHE BOOL "Use platform-specific keywords and modifiers to ensure a function being inlined.")

if (BLAZE_INLINE_ALWAYS)
   set(BLAZE_INLINE_ALWAYS 1)
   set(BLAZE_INLINE_STRONG 1)
else ()
   set(BLAZE_INLINE_ALWAYS 0)
   set(BLAZE_INLINE_STRONG 0)
endif ()

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/Inline.h.in"
                "${PROJECT_SOURCE_DIR}/blaze/config/Inline.h")

#==================================================================================================
# Installation
#==================================================================================================

include(FetchContent)
FetchContent_Declare(twCMakeUtils
        GIT_REPOSITORY https://github.com/timow-gh/twCMakeUtils.git
        GIT_TAG main)
FetchContent_GetProperties(twCMakeUtils)
set(lcs_twCMakeUtils "")
string(TOLOWER "twCMakeUtils" lcs_twCMakeUtils)
if (NOT ${lcs_twCMakeUtils}_POPULATED)
   FetchContent_Populate(twCMakeUtils)
   list(APPEND CMAKE_MODULE_PATH ${${lcs_twCMakeUtils}_SOURCE_DIR}/modules)
   include(CMakeHelperFunctions)
endif ()

if (${PROJECT_NAME}_INSTALL)
   install(DIRECTORY ${PROJECT_SOURCE_DIR}/blaze DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
   install(TARGETS blaze
           EXPORT blazeTargetsExportSet
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

   setup_export_set_installation(
           "blaze"
           "blazeTargetsExportSet"
           Config.cmake.in
           ${PROJECT_VERSION_MAJOR} ${PROJECT_VERSION_MINOR} ${PROJECT_VERSION_PATCH}
   )

   export(EXPORT blazeTargetsExportSet NAMESPACE blaze:: FILE ${CMAKE_BINARY_DIR}/blazeTargets.cmake)
endif ()

