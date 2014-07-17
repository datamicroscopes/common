cmake_minimum_required(VERSION 2.6)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(microscopes_common)

set(CMAKE_CXX_FLAGS_BASE "${CMAKE_CXX_FLAGS} -fPIC -g -MD -Wall -std=c++0x")

if(APPLE)
  set(CMAKE_CXX_FLAGS_BASE "${CMAKE_CXX_FLAGS_BASE} -stdlib=libc++")
  # for anaconda builds
  set(CMAKE_CXX_FLAGS_BASE "${CMAKE_CXX_FLAGS_BASE} -mmacosx-version-min=10.7")
  # clang complains about register
  set(CMAKE_CXX_FLAGS_BASE "${CMAKE_CXX_FLAGS_BASE} -Wno-deprecated-register")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_BASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_BASE} -DDEBUG_MODE")

if(DEFINED EXTRA_INCLUDE_PATH)
  include_directories(${EXTRA_INCLUDE_PATH})
endif()

find_package(Protobuf REQUIRED)
message(STATUS "found protobuf INC=${PROTOBUF_INCLUDE_DIRS}, LIB=${PROTOBUF_LIBRARY_DIRS}")
include_directories(${PROTOBUF_INCLUDE_DIRS})
link_directories(${PROTOBUF_LIBRARY_DIRS})

find_package(Distributions)
if(DISTRIBUTIONS_FOUND)
  message(STATUS "found distributions INC=${DISTRIBUTIONS_INCLUDE_DIRS}, LIB=${DISTRIBUTIONS_LIBRARY_DIRS}")
  include_directories(${DISTRIBUTIONS_INCLUDE_DIRS})
  link_directories(${DISTRIBUTIONS_LIBRARY_DIRS})
else()
  message(FATAL_ERROR "Could not find distributions")
endif()

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/src/io/schema.pb.cpp ${CMAKE_CURRENT_BINARY_DIR}/include/microscopes/io/schema.pb.h
    COMMAND cmake/generate_protobuf.sh ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/microscopes/io/schema.proto)

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h*")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ DESTINATION include FILES_MATCHING PATTERN "*.h*")
install(DIRECTORY microscopes DESTINATION cython FILES_MATCHING PATTERN "*.pxd"  PATTERN "__init__.py")

set(MICROSCOPES_COMMON_SOURCE_FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/src/io/schema.pb.cpp
    src/common/assert.cpp
    src/common/group_manager.cpp
    src/common/recarray/dataview.cpp
    src/common/runtime_type.cpp
    src/common/runtime_value.cpp
    src/common/sparse_ndarray/dataview.cpp
    src/common/util.cpp
    src/models/bbnc.cpp
    src/models/distributions.cpp
    src/models/niw.cpp)
add_library(microscopes_common SHARED ${MICROSCOPES_COMMON_SOURCE_FILES})
target_link_libraries(microscopes_common protobuf distributions_shared)
install(TARGETS microscopes_common LIBRARY DESTINATION lib)

# test executables
enable_testing()
add_executable(test_sparse_ndarray test/cxx/test_sparse_ndarray.cpp)
add_test(test_sparse_ndarray test_sparse_ndarray)
target_link_libraries(test_sparse_ndarray protobuf distributions_shared microscopes_common)
