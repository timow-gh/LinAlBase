add_executable(linAlBase_blazeExample blazeExample.cpp)
setMSVCOutputDirectories(linAlBase_blazeExample)
target_link_libraries(linAlBase_blazeExample PRIVATE blaze::blaze)
install(TARGETS linAlBase_blazeExample
        EXPORT LinAlBaseTargetsExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})