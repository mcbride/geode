cmake_minimum_required(VERSION 3.5)
project(geode)

option(GEODE_REQUIRE_PYTHON "Generate an error if python bindings won't be built instead of silently disabling them" FALSE)
option(GEODE_DISABLE_PYTHON "Don't build python bindings even if available" FALSE)

if (GEODE_REQUIRE_PYTHON AND GEODE_DISABLE_PYTHON)
  message(STATUS "Geode Python bindings set to required and disabled at the same time. Please choose at most one")
endif()

include(GeodeSupport.cmake)

find_package(PkgConfig)
find_package(JPEG)
find_package(PNG)

find_library(
  GMP_LIBRARIES gmp
  HINTS /opt/homebrew/lib /usr/local/lib /usr/lib
)
find_path(
  GMP_INCLUDE gmp.h
  HINTS /opt/homebrew/include /usr/local/include /usr/include
)
if (GMP_LIBRARIES AND GMP_INCLUDE)
  set(GMP_FOUND TRUE)
endif()

if (GMP_FOUND)
  message(STATUS "GMP found in ${GMP_LIBRARIES}")
else()
  message(FATAL_ERROR "Required dependency GMP not found. Set GMP_LIB_DIR and GMP_INCLUDE_DIR to inform cmake where to look")
endif()



find_library(
  OPENMESH_LIBRARIES OpenMeshCore
  HINTS /opt/homebrew/lib /usr/local/lib /usr/local/lib/OpenMesh
)

find_path(
  OPENMESH_INCLUDE OpenMesh/Core/Mesh/TriMeshT.hh
  HINTS /opt/homebrew/include /usr/local/include /usr/include
)

if (OPENMESH_LIBRARIES AND OPENMESH_INCLUDE)
  set(OPENMESH_FOUND TRUE)
endif()

if (OPENMESH_FOUND)
  message(STATUS "OPENMESH found in ${OPENMESH_LIBRARIES}")
else()
  message(FATAL_ERROR "Required dependency OPENMESH not found. Set OPENMESH_LIB_DIR and OPENMESH_INCLUDE_DIR to inform cmake where to look")
endif()


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/)

find_package(PathPython)

add_subdirectory(geode)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/geode.pc.in ${CMAKE_BINARY_DIR}/geode.pc
  @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/geode.pc DESTINATION lib/pkgconfig/)

if(NOT GUI_DIR)
  # Check in current directory by default
  set(GUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gui)
endif()

if (EXISTS ${GUI_DIR})
  message(STATUS "gui found at ${GUI_DIR}")
  set(Geode_DIR ${CMAKE_CURRENT_BINARY_DIR}/geode)
  add_subdirectory(${GUI_DIR} ${CMAKE_CURRENT_BINARY_DIR}/gui)
endif()
