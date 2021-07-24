cmake_minimum_required(VERSION 3.19)

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
endif()

project(LinAlBase
        VERSION 0.1.0
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(LinAlBase_BUILD_EXAMPLES "" ON)

preamble()

FetchContent_Declare(
        OpenBLAS
        GIT_REPOSITORY https://github.com/timow-gh/OpenBLASWrapper.git
        GIT_TAG main)
FetchContent_MakeAvailable(OpenBLAS)


FetchContent_Declare(
        blaze
        GIT_REPOSITORY https://bitbucket.org/blaze-lib/blaze.git
        GIT_TAG v3.8)
FetchContent_GetProperties(blaze)
if (NOT blaze_POPULATED)
    option(USE_LAPACK "" ON)
    set(BLAZE_BLAS_MODE ON CACHE BOOL "Enables/Disable the BLAS mode.")
    if (BLAZE_BLAS_MODE)
        set(BLAZE_BLAS_IS_64BIT 1 CACHE BOOL "Specify whether the used BLAS library itself is 64-bit or not.")
        set(BLAZE_BLAS_IS_PARALLEL 0 CACHE BOOL "Specify whether the used BLAS library is itself parallelized or not.")
        set(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION 0 CACHE BOOL "Use BLAS kernels for dense matrix/vector multiplications instead of the default Blaze kernels.")
        set(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION 1 CACHE BOOL "Use BLAS kernels for dense matrix multiplications instead of the default Blaze kernels.")
        set(BLAZE_BLAS_INCLUDE_FILE "<openblas/cblas.h>" CACHE STRING "Specify the name of the BLAS header file (e.g. with MKL the file is called mkl_cblas.h)")
    endif ()

    FetchContent_Populate(blaze)
    file(REMOVE ${blaze_SOURCE_DIR}/CMakeLists.txt)
    file(COPY ${PROJECT_SOURCE_DIR}/cmake/blazeCMakeListsFile/CMakeLists.txt DESTINATION ${blaze_SOURCE_DIR})
    add_subdirectory(${blaze_SOURCE_DIR} ${blaze_BINARY_DIR})
    install(TARGETS blaze
            EXPORT LinAlBaseTargetsExportSet
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    add_library(blaze::blaze ALIAS blaze)
endif ()

add_library(ILinAlBase INTERFACE)
target_link_libraries(ILinAlBase INTERFACE OpenBLAS::OpenBLAS blaze::blaze)
set_target_properties(ILinAlBase PROPERTIES EXPORT_NAME LinAlBase)
add_library(LinAlBase::LinAlBase ALIAS ILinAlBase)
install(TARGETS ILinAlBase
        EXPORT LinAlBaseTargetsExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
setupExportSetInstall(${PROJECT_NAME} "LinAlBaseTargetsExportSet")

if (LinAlBase_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

