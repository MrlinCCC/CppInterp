# CMakeList.txt: CppInterp 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CppInterp")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 将源代码添加到此项目的可执行文件。
add_library(CppInterpLib
   src/Lexer.cpp
   src/Parser.cpp
)

add_executable(CppInterp
    src/main.cpp
)

# TODO: 如有需要，请添加测试并安装目标。
add_subdirectory(UnitTest)
add_subdirectory(Common)
target_include_directories(CppInterpLib PUBLIC include)
target_link_libraries(CppInterpLib PUBLIC Common)
target_link_libraries(CppInterp PRIVATE CppInterpLib)
