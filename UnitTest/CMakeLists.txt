add_executable(Test RunTest.cpp)

find_package(GTest CONFIG REQUIRED)

target_include_directories(Test PRIVATE .)

target_link_libraries(Test PRIVATE GTest::gtest GTest::gtest_main CppInterpLib)