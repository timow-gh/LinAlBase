add_executable(linAlBase_openBLASExample openBLASExample.cpp)
target_link_libraries(linAlBase_openBLASExample PRIVATE OpenBLAS::OpenBLAS)
setMSVCOutputDirectories(linAlBase_openBLASExample)
install(TARGETS linAlBase_openBLASExample
        EXPORT LinAlBaseTargetsExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
