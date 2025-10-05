find_package(GTest)
if(NOT GTest_FOUND)
    message(STATUS "GTest not found, using FetchContent")
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
endif()

set(
    TEST_FILES
    "BufferedPageTest"
    "PacketHeaderTest"
    "CropStreambufTest"
    "PacketBuilderTest"
    "PacketReaderTest"
    "ProtocolTest"
    "ScoaStreambufTest"
    "ScoaStateTest"
    "StreamPacketTest"
)

foreach(file ${TEST_FILES})
    add_executable(${file} ${file}.cpp)
    target_link_libraries(${file} PRIVATE GTest::gtest_main GTest::gmock libcapt)
    gtest_discover_tests(${file})
endforeach()
