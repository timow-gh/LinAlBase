add_executable(example example.cpp)
target_link_libraries(example PRIVATE OpenBLAS::OpenBLAS)
setMSVCOutputDirectories(example)
install(TARGETS example
        EXPORT OpenBLASTargetsExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
