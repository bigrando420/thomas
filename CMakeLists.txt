cmake_minimum_required(VERSION 3.11)

project(thomas)

find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

set(PROJECT_SOURCE_DIR sauce)
set(PROJECT_DATA_DIR data)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCES ${PROJECT_SOURCE_DIR}/anvil.cpp
  ${PROJECT_SOURCE_DIR}/third_party/telescope_light.c)
file(GLOB DATA ${PROJECT_DATA_DIR}/*)

set_source_files_properties(
  ${PROJECT_SOURCE_DIR}/third_party/telescope_light.c
  PROPERTIES LANGUAGE CXX
)

configure_file(${DATA} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
  ${OPENGL_LIBRARIES}
  ${X11_LIBRARIES}
  ${X11_Xcursor_LIB}
  ${X11_Xi_LIB}
  "-ldl")

target_include_directories(${PROJECT_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR})
