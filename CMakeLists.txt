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
    set(utils_cmake_modules_path ${${lcs_twCMakeUtils}_SOURCE_DIR}/modules)
    list(APPEND CMAKE_MODULE_PATH ${utils_cmake_modules_path})
endif ()

project(LinAlBase
        VERSION 0.1.0
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(setup_export_set_installation)
include(project_preamble)
project_preamble()

if ((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR))
    set(IS_TOP_LEVEL_PROJECT TRUE)
else ()
    set(IS_TOP_LEVEL_PROJECT FALSE)
endif ()

if (IS_TOP_LEVEL_PROJECT)
    option(LinAlBase_BUILD_EXAMPLES "" ON)
else ()
    option(LinAlBase_BUILD_EXAMPLES "" OFF)
endif ()
option(LinAlBase_USE_OPENBLAS "Experimental use of blaze with OpenBLAS" OFF)
option(${PROJECT_NAME}_INSTALL "Install project targets" ON)

if (LinAlBase_USE_OPENBLAS)
    FetchContent_Declare(
            OpenBLAS
            GIT_REPOSITORY https://github.com/timow-gh/OpenBLASWrapper.git
            GIT_SHALLOW TRUE
            GIT_TAG main)
    FetchContent_MakeAvailable(OpenBLAS)
endif ()

FetchContent_Declare(
        blaze
        GIT_REPOSITORY https://bitbucket.org/blaze-lib/blaze.git
        GIT_SHALLOW TRUE
        GIT_TAG v3.8)
FetchContent_GetProperties(blaze)
if (NOT blaze_POPULATED)
    option(USE_LAPACK "" OFF)
    set(BLAZE_BLAS_MODE OFF CACHE BOOL "Enables/Disable the BLAS mode.")
    set(BLAZE_BLAS_IS_64BIT 1 CACHE BOOL "Specify whether the used BLAS library itself is 64-bit or not." FORCE)
    set(BLAZE_BLAS_IS_PARALLEL 0 CACHE BOOL "Specify whether the used BLAS library is itself parallelized or not." FORCE)
    set(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION 0 CACHE BOOL "Use BLAS kernels for dense matrix/vector multiplications instead of the default Blaze kernels." FORCE)
    set(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION 0 CACHE BOOL "Use BLAS kernels for dense matrix multiplications instead of the default Blaze kernels." FORCE)
    if (LinAlBase_USE_OPENBLAS)
        set(USE_LAPACK ON CACHE BOOL "" FORCE)
        set(BLAZE_BLAS_MODE ON CACHE BOOL "Enables/Disable the BLAS mode." FORCE)
        set(BLAZE_BLAS_IS_64BIT 1 CACHE BOOL "Specify whether the used BLAS library itself is 64-bit or not." FORCE)
        set(BLAZE_BLAS_IS_PARALLEL 0 CACHE BOOL "Specify whether the used BLAS library is itself parallelized or not." FORCE)
        set(BLAZE_BLAS_USE_MATRIX_VECTOR_MULTIPLICATION 0 CACHE BOOL "Use BLAS kernels for dense matrix/vector multiplications instead of the default Blaze kernels." FORCE)
        set(BLAZE_BLAS_USE_MATRIX_MATRIX_MULTIPLICATION 1 CACHE BOOL "Use BLAS kernels for dense matrix multiplications instead of the default Blaze kernels." FORCE)
        set(BLAZE_BLAS_INCLUDE_FILE "<openblas/cblas.h>" CACHE STRING "Specify the name of the BLAS header file (e.g. with MKL the file is called mkl_cblas.h)" FORCE)
    endif ()

    FetchContent_Populate(blaze)
    if (LinAlBase_USE_OPENBLAS)
        file(REMOVE ${blaze_SOURCE_DIR}/CMakeLists.txt)
        file(COPY ${PROJECT_SOURCE_DIR}/cmake/blazeCMakeListsFile/CMakeLists.txt DESTINATION ${blaze_SOURCE_DIR})
        file(REMOVE ${blaze_SOURCE_DIR}/Config.cmake.in)
        file(REMOVE ${blaze_SOURCE_DIR}/cmake/blaze-config.cmake)
        file(COPY ${PROJECT_SOURCE_DIR}/cmake/blazeCMakeListsFile/OpenBLASConfig.cmake.in DESTINATION ${blaze_SOURCE_DIR}/cmake)
    else ()
        file(REMOVE ${blaze_SOURCE_DIR}/CMakeLists.txt)
        file(COPY ${PROJECT_SOURCE_DIR}/cmake/blazeCMakeListsFile/CMakeLists.txt DESTINATION ${blaze_SOURCE_DIR})
        file(REMOVE ${blaze_SOURCE_DIR}/Config.cmake.in)
        file(REMOVE ${blaze_SOURCE_DIR}/cmake/blaze-config.cmake)
        file(COPY ${PROJECT_SOURCE_DIR}/cmake/blazeCMakeListsFile/Config.cmake.in DESTINATION ${blaze_SOURCE_DIR}/cmake)
    endif ()
    add_subdirectory(${blaze_SOURCE_DIR} ${blaze_BINARY_DIR})
    add_library(blaze::blaze ALIAS blaze)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(blaze INTERFACE
                "-Wno-reserved-id-macro"
                "-Wno-undef"
                "-Wno-documentation-unknown-command"
                "-Wno-extra-semi-stmt"
                "-Wno-sign-conversion"
                "-Wno-implicit-int-float-conversion"
                "-Wno-double-promotion"
                "-Wno-implicit-float-conversion"
                )
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        target_compile_options(blaze INTERFACE
                "-Wno-undef"
                "-Wno-double-promotion"
                )
    else ()
        message(WARNING "Compiler warnings for blaze are not set.")
    endif ()
endif ()

add_library(ILinAlBase INTERFACE)
target_link_libraries(ILinAlBase INTERFACE blaze::blaze)
target_include_directories(ILinAlBase
        INTERFACE
        $<BUILD_INTERFACE:${blaze_SOURCE_DIR}>
        $<BUILD_INTERFACE:${openblas_SOURCE_DIR}/include>)
set_target_properties(ILinAlBase PROPERTIES EXPORT_NAME LinAlBase)
add_library(LinAlBase::LinAlBase ALIAS ILinAlBase)

if (LinAlBase_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (${PROJECT_NAME}_INSTALL)
    install(TARGETS ILinAlBase blaze
            EXPORT LinAlBaseTargetsExportSet
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    setup_export_set_installation(
            "${PROJECT_NAME}"
            "LinAlBaseTargetsExportSet"
            Config.cmake.in
            ${PROJECT_VERSION_MAJOR} ${PROJECT_VERSION_MINOR} ${PROJECT_VERSION_PATCH}
    )
endif ()