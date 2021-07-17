cmake_minimum_required(VERSION 3.19)
project(LinAlBase
        VERSION 0.1.0
        LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(BUILD_SHARED_LIBS ON)
string(TOLOWER "${CMAKE_BUILD_TYPE}" lc_build_type)
set(CMAKE_INSTALL_PREFIX C:/Users/timo/dev/repos/cmake_prefix_dir_${lc_build_type})

include(HelperFunctions)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (NOT CMAKE_GENERATOR STREQUAL "Xcode")
    file(RELATIVE_PATH relDir
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
            ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
    set(CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${relDir})
endif ()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

include(FetchContent)

if (MSVC)
    FetchContent_Declare(
            external_openblas
            URL https://github.com/xianyi/OpenBLAS/releases/download/v0.3.16/OpenBLAS-0.3.16-x64.zip
            URL_HASH MD5=2f4404a4da21b319447b3ce7fe351426)
    FetchContent_GetProperties(external_openblas)
    string(TOLOWER "external_openblas" lcOpenBLASName)
    if (NOT ${lcOpenBLASName}_POPULATED)
        FetchContent_Populate(external_openblas)

        execute_process(COMMAND ${CMAKE_COMMAND} -E rm -R -f ${${lcOpenBLASName}_SOURCE_DIR}/lib/pkgconfig)
        execute_process(COMMAND ${CMAKE_COMMAND} -E rm -R -f ${${lcOpenBLASName}_SOURCE_DIR}/lib/cmake)
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${${lcOpenBLASName}_SOURCE_DIR}/include/openblas)
        set(OPENBLAS_INCLUDE_FILES "")
        file(GLOB OPENBLAS_INCLUDE_FILES ${${lcOpenBLASName}_SOURCE_DIR}/include/*.h)
        foreach (includeFile ${OPENBLAS_INCLUDE_FILES})
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${includeFile} ${${lcOpenBLASName}_SOURCE_DIR}/include/openblas)
            execute_process(COMMAND ${CMAKE_COMMAND} -E rm ${includeFile})
        endforeach ()

        add_library(OpenBLAS INTERFACE)
        add_library(OpenBLAS::OpenBLAS ALIAS OpenBLAS)

        target_include_directories(OpenBLAS
                INTERFACE
                $<BUILD_INTERFACE:${${lcOpenBLASName}_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

        target_link_libraries(OpenBLAS INTERFACE
                "$<BUILD_INTERFACE:${${lcOpenBLASName}_SOURCE_DIR}/lib/libopenblas.lib>"
                )
        target_link_libraries(OpenBLAS INTERFACE
                "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/lib/libopenblas.lib>"
                )

        install(DIRECTORY "${${lcOpenBLASName}_SOURCE_DIR}/include" DESTINATION "${CMAKE_INSTALL_PREFIX}")
        install(FILES "${${lcOpenBLASName}_SOURCE_DIR}/bin/libopenblas.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
        install(FILES "${${lcOpenBLASName}_SOURCE_DIR}/lib/libopenblas.lib" DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
#        install(TARGETS OpenBLAS
#                EXPORT LinAlBaseTargetsExportSet
#                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#                LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#                INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

        if ((CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR))
            if (NOT CMAKE_BUILD_TYPE)
                message(FATAL_ERROR "CMAKE_BUILD_TYPE not defined.")
            endif ()
            execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
            execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${${lcOpenBLASName}_SOURCE_DIR}/bin/libopenblas.dll "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
        endif ()
    endif ()
endif ()

FetchContent_Declare(
        blaze
        GIT_REPOSITORY https://bitbucket.org/blaze-lib/blaze.git
        GIT_TAG v3.8)
FetchContent_GetProperties(blaze)
string(TOLOWER "blaze" lcBlazeName)
if (NOT ${lcBlazeName}_POPULATED)
    set(USE_LAPACK OFF)
    FetchContent_Populate(blaze)
    add_subdirectory(${${lcBlazeName}_SOURCE_DIR} ${${lcBlazeName}_BINARY_DIR})
    add_library(blaze::blaze ALIAS blaze)
endif ()

add_library(ILinAlBase INTERFACE)
target_link_libraries(ILinAlBase INTERFACE OpenBLAS::OpenBLAS blaze::blaze)
set_target_properties(ILinAlBase PROPERTIES EXPORT_NAME LinAlBase)
add_library(LinAlBase::LinAlBase ALIAS ILinAlBase)

install(TARGETS ILinAlBase OpenBLAS
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
        CACHE STRING "Path to install ${PROJECT_NAME}'s .cmake  module files to.")

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
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR})

add_subdirectory(examples)
