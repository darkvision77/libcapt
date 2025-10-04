add_executable(printpbm printpbm.cpp)
target_link_libraries(printpbm PRIVATE libcapt)

add_executable(getstatus getstatus.cpp)
target_link_libraries(getstatus PRIVATE libcapt)

add_executable(printerinfo printerinfo.cpp)
target_link_libraries(printerinfo PRIVATE libcapt)
