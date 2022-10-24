cmake_minimum_required(VERSION 3.11)

project(thomas)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

set(PROJECT_SOURCE_DIR sauce)
set(PROJECT_DATA_DIR data)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB DATA ${PROJECT_DATA_DIR}/*)

configure_file(${DATA} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
  ${OPENGL_LIBRARIES}
  ${X11_LIBRARIES}
  ${X11_Xcursor_LIB}
  ${X11_Xi_LIB}
  "-ldl")
