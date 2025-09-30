add_library(Common STATIC 
    src/Logger.cpp 
    src/ThreadPool.cpp 
    src/Semaphore.cpp
    src/Timestamp.cpp
    src/Spinlock.cpp
    src/Timer.cpp
    )

find_package(fmt CONFIG REQUIRED)
target_include_directories(Common PUBLIC include)
target_link_libraries(Common PUBLIC fmt::fmt)