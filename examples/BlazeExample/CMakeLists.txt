add_executable(linAlBase_blazeExample blazeExample.cpp)
target_link_libraries(linAlBase_blazeExample PRIVATE blaze::blaze)
install(TARGETS linAlBase_blazeExample
        EXPORT linAlBase_example_ExportSet
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})