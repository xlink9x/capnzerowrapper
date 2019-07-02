cmake_minimum_required(VERSION 2.8.3)
project(capnzerowrapper)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

# Define where to find modules for ZeroMQ
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL "" FORCE)

find_package(catkin REQUIRED COMPONENTS
        capnzero
        )

# copying the variables makes it compatible for catkin to transitive forward the CapnProto libraries and include dirs
set(CapnProto_LIBRARIES ${CAPNP_LIBRARIES})
set(CapnProto_INCLUDE_DIRS ${CAPNP_INCLUDE_DIRS})

## System dependencies are found with CMake's conventions
find_package(ZeroMQ REQUIRED)
find_package(CapnProto REQUIRED)

catkin_package(
        INCLUDE_DIRS include
        LIBRARIES capnzerowrapper
        CATKIN_DEPENDS capnzero
        DEPENDS ZeroMQ CapnProto
        CFG_EXTRAS CapnZero-extras.cmake
)
include_directories(
        include
        ${catkin_INCLUDE_DIRS}
        ${ZeroMQ_INCLUDE_DIRS}
        ${CapnProto_INCLUDE_DIRS}
)


## Declare a C++ library
add_library(${PROJECT_NAME}
        src/capnzerowrapper.cpp
        include/capnzerowrapper.h
        )
target_link_libraries(${PROJECT_NAME}
        ${catkin_LIBRARIES}
        ${ZeroMQ_LIBRARIES}
        ${CAPNP_LIBRARIES}
        )
if (NOT catkin_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
endif (NOT catkin_FOUND)

add_executable(${PROJECT_NAME}_main
        src/capnzerowrapper.cpp)

target_link_libraries(${PROJECT_NAME}_main
        PUBLIC
        ${catkin_LIBRARIES}
        ${ZeroMQ_LIBRARIES}
        ${CAPNP_LIBRARIES}
        )

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(${PROJECT_NAME} PUBLIC ZMQ_BUILD_DRAFT_API=1)
