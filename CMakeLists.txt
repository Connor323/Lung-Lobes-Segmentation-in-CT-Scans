# This is the root ITK CMakeLists file.
cmake_minimum_required(VERSION 2.8.9)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif()


# This project is designed to be built outside the Insight source tree.
project(HelloWorld)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(vector_region_growing vector_region_growing.cxx)
target_link_libraries(vector_region_growing ${ITK_LIBRARIES})

add_executable(region_growing region_growing.cxx)
target_link_libraries(region_growing ${ITK_LIBRARIES})