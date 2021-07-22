cmake_minimum_required(VERSION 3.19)
project(LinAlBase
        VERSION 0.1.0
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(HelperFunctions)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

set(BUILD_SHARED_LIBS ON)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

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

set(${PROJECT_NAME}_INSTALL_CMAKEDIR
        "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        CACHE STRING "Path to install ${PROJECT_NAME} Config*.cmake files to.")
set(${PROJECT_NAME}_MODULE_INSTALL_DIR
        "${CMAKE_INSTALL_LIBDIR}/cmake"
        CACHE STRING "Path to install ${PROJECT_NAME}'s .cmake module files to.")

install(EXPORT LinAlBaseTargetsExportSet
        NAMESPACE LinAlBase::
        FILE LinAlBaseTargets.cmake
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR})

write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
        COMPATIBILITY SameMajorVersion)

configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR}
        PATH_VARS ${PROJECT_NAME}_MODULE_INSTALL_DIR
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake "
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR})

add_subdirectory(examples)

