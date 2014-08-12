cmake_minimum_required(VERSION 2.6)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(microscopes_common)

# default mode is RelWithDebInfo
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -MD -std=c++0x")

# since we use distributions headers, we need to inherit their
# no-strict-aliasing warnings rule otherwise deal with compiler noise
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-strict-aliasing")

# this warning is annoying; who cares about unused parameters
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  # for anaconda builds
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.7")
  # clang complains about register
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")
  # OS X doesn't have librt
  set(RT_LIBRARY_NAME "")
else()
  set(RT_LIBRARY_NAME "rt")
endif()

# taken from distributions
#set(CMAKE_CXX_FLAGS_MATHOPT "-mfpmath=sse -msse4.1 -ffast-math -funsafe-math-optimizations")
set(CMAKE_CXX_FLAGS_MATHOPT "-mfpmath=sse -msse4.1")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG ${CMAKE_CXX_FLAGS_MATHOPT}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG_MODE -fno-omit-frame-pointer")

# give our include dirs the most precedent
include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# followed by the EXTRA_* ones
if(DEFINED EXTRA_INCLUDE_PATH)
  include_directories(${EXTRA_INCLUDE_PATH})
endif()
if(DEFINED EXTRA_LIBRARY_PATH)
  link_directories(${EXTRA_LIBRARY_PATH})
endif()

# then our packages
find_package(Protobuf REQUIRED)
message(STATUS "found protobuf INC=${PROTOBUF_INCLUDE_DIRS}, LIB=${PROTOBUF_LIBRARIES}")
include_directories(${PROTOBUF_INCLUDE_DIRS})

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
    src/common/relation/dataview.cpp
    src/common/util.cpp
    src/common/scalar_functions.cpp
    src/models/bbnc.cpp
    src/models/distributions.cpp
    src/models/dm.cpp
    src/models/noop.cpp)
add_library(microscopes_common SHARED ${MICROSCOPES_COMMON_SOURCE_FILES})
target_link_libraries(microscopes_common ${PROTOBUF_LIBRARIES} distributions_shared)
install(TARGETS microscopes_common LIBRARY DESTINATION lib)

# bin executables
add_executable(perf_group bin/perf_group.cpp)
target_link_libraries(perf_group ${PROTOBUF_LIBRARIES} distributions_shared microscopes_common ${RT_LIBRARY_NAME})

# test executables
enable_testing()
add_executable(test_relation test/cxx/test_relation.cpp)
add_executable(test_group_manager test/cxx/test_group_manager.cpp)
add_executable(test_headers test/cxx/test_headers.cpp)
add_test(test_relation test_relation)
add_test(test_group_manager test_group_manager)
add_test(test_headers test_headers)
target_link_libraries(test_relation ${PROTOBUF_LIBRARIES} distributions_shared microscopes_common)
target_link_libraries(test_group_manager ${PROTOBUF_LIBRARIES} distributions_shared microscopes_common)
target_link_libraries(test_headers ${PROTOBUF_LIBRARIES} distributions_shared microscopes_common)
