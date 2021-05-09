cmake_minimum_required(VERSION 3.4.1)

# LIB目录和BIN目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/${ANDROID_ABI})

#设置头文件搜索路径（和此txt同个路径的头文件无需设置），可选
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/common)

#指定用到的系统库或者NDK库或者第三方库的搜索路径，可选。
#LINK_DIRECTORIES(/usr/local/lib)

#添加子目录，将自动找到子目录中的CMakeLists.txt
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/main/cpp/native-lib)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/main/cpp/PtraceInject)
ADD_SUBDIRECTORY(${PROJECT_SOURCE_DIR}/src/main/cpp/InjectModule)