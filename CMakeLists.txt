cmake_minimum_required(VERSION 3.0)

project(FreeCloud)

add_definitions("-std=c++11")
aux_source_directory(./src DIR_SRC)

include_directories(${PROJECT_SOURCE_DIR}/third_party/glog/include)
include_directories(${PROJECT_SOURCE_DIR}/third_party/gflags/include)

link_directories(${PROJECT_SOURCE_DIR}/third_party/gflags/lib)
link_directories(${PROJECT_SOURCE_DIR}/third_party/glog/lib)

add_executable(FreeCloud ${DIR_SRC})
target_link_libraries(FreeCloud libglog.a libgflags.a -lpthread)
