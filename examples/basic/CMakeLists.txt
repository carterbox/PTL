#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(PTL_Basic_Examples LANGUAGES C CXX)


#----------------------------------------------------------------------------
# Find PTL project
#
find_package(PTL REQUIRED)
set(EXTERNAL_INCLUDE_DIRS ${PTL_INCLUDE_DIRS})
set(EXTERNAL_LIBRARIES ${PTL_LIBRARIES})
include_directories(${EXTERNAL_INCLUDE_DIRS})
if(NOT DEFINED common_headers)
    get_filename_component(COMMON_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
    include_directories(${COMMON_DIR})
    file(GLOB common_headers ${COMMON_DIR}/*.hh)
endif()


#----------------------------------------------------------------------------
# Source files
#
file(GLOB headers ${CMAKE_CURRENT_LIST_DIR}/*.h ${CMAKE_CURRENT_LIST_DIR}/*.hh)


#----------------------------------------------------------------------------
# standard example
#
add_executable(tasking tasking.cc ${headers} ${common_headers})
target_link_libraries(tasking ${EXTERNAL_LIBRARIES})
set_target_properties(tasking PROPERTIES COMPILE_FLAGS ${PTL_CXX_FLAGS})

if(PTL_USE_TBB)
    add_executable(tbb_tasking tasking.cc ${headers})
    set_target_properties(tbb_tasking PROPERTIES
        COMPILE_DEFINITIONS USE_TBB_TASKS
        COMPILE_FLAGS ${PTL_CXX_FLAGS})
    target_link_libraries(tbb_tasking ${EXTERNAL_LIBRARIES})
endif(PTL_USE_TBB)


#----------------------------------------------------------------------------
# recursive example
#
add_executable(recursive_tasking recursive_tasking.cc ${headers} ${common_headers})
target_link_libraries(recursive_tasking ${EXTERNAL_LIBRARIES})
set_target_properties(recursive_tasking PROPERTIES COMPILE_FLAGS ${PTL_CXX_FLAGS})

if(PTL_USE_TBB)
    add_executable(recursive_tbb_tasking recursive_tasking.cc ${headers} ${common_headers})
    set_target_properties(recursive_tbb_tasking PROPERTIES
        COMPILE_DEFINITIONS USE_TBB_TASKS
        COMPILE_FLAGS ${PTL_CXX_FLAGS})
    target_link_libraries(recursive_tbb_tasking ${EXTERNAL_LIBRARIES})
endif(PTL_USE_TBB)


#----------------------------------------------------------------------------
# installation
install(TARGETS tasking recursive_tasking DESTINATION bin)
if(PTL_USE_TBB)
    install(TARGETS recursive_tasking recursive_tbb_tasking DESTINATION bin)
endif(PTL_USE_TBB)
