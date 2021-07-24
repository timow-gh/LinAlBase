add_executable(linAlBase_openBLASExample openBLASExample.cpp)
target_link_libraries(linAlBase_openBLASExample PRIVATE OpenBLAS::OpenBLAS)
install(TARGETS linAlBase_openBLASExample
        EXPORT linAlBase_example_ExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
