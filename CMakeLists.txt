cmake_minimum_required(VERSION 3.27)
set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build for x86_64 only" FORCE)

project(eu4dll_mac)

set(CMAKE_CXX_STANDARD 17)

add_library(eu4dll_mac SHARED library.cpp library.h
        src/base.cpp
        src/base.h
        src/mainText.cpp
        src/mainText.h
        src/tooltipAndButtonText.cpp
        src/tooltipAndButtonText.h
        src/mapText.cpp
        src/mapText.h
        src/text3D.cpp
        src/text3D.h
        src/textLayout.cpp
        src/textLayout.h
        src/saveFileName.cpp
        src/saveFileName.h
        src/strConvert.cpp
        src/strConvert.h
        src/input.cpp
        src/input.h
        src/memoryHelper.cpp
        src/memoryHelper.h
        src/localization.cpp
        src/localization.h
        src/pinyinHelper.cpp
        src/pinyinHelper.h)

include(FetchContent)
FetchContent_Declare(
        cpp-pinyin
        GIT_REPOSITORY https://github.com/wolfgitpr/cpp-pinyin.git
        GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(cpp-pinyin)
target_link_libraries(eu4dll_mac PRIVATE cpp-pinyin)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${cpp-pinyin_SOURCE_DIR}/res/dict"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/chinese_dict"
)

find_library(CORE_FOUNDATION_LIB CoreFoundation)
target_link_libraries(eu4dll_mac PRIVATE ${CORE_FOUNDATION_LIB})

add_subdirectory(tool)