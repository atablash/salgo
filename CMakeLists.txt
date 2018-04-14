cmake_minimum_required (VERSION 3.0)
project (salgo)

if(NOT CMAKE_BUILD_TYPE) 
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)









#===============================================================================
# BUILD OPTIONS
#===============================================================================
option(SALGO_BUILD_TESTS "Build salgo tests" ON) # TODO: set to off
option(SALGO_BUILD_BENCHMARKS "Build salgo benchmarks" ON) # TODO: set to off
option(SALGO_USE_LIBCXX "Link with libc++ (instead of default, usually libstdc++)" OFF) # TODO: set to off







#===============================================================================
# Additional cmake modules
#===============================================================================
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#set(CMAKE_CXX_STANDARD 17) # not supported by older cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fno-exceptions")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pipe -march=native -mtune=native -mfpmath=sse")






#===============================================================================
# set compiler options, output all warnings
#===============================================================================
add_definitions(
	-Wall -Wextra -Werror -Wshadow
)



if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	message(STATUS "Detected clang")
	if(SALGO_USE_LIBCXX)
		SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message(STATUS "Detected gcc")
	if(SALGO_USE_LIBCXX)
		SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -nostdinc++ -I/usr/local/include/c++/v1" )
		link_libraries(-nodefaultlibs -lc++ -lc++abi -lm -lc -lgcc_s -lgcc)
		
		# required by GTest
		set(THREADS_PREFER_PTHREAD_FLAG ON)
		find_package(Threads REQUIRED)
	endif()
else()
	message(WARNING "COMPILER " ${} " IS UNSUPPORTED BY SALGO!")
	if(SALGO_USE_LIBCXX)
		SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
	endif()
endif()






#===============================================================================
# packages
#===============================================================================
find_package(glog REQUIRED)
if (GLOG_FOUND)
	link_libraries(glog)
endif ()


find_package(Eigen3 3.2 REQUIRED)
if (EIGEN3_FOUND)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIRS})
endif()






# TODO: remove tinyply dependency
set(sources ${sources} ../third-party/tinyply/source/tinyply.cpp)
#add_definitions(-Wno-unused-parameter)







#===============================================================================
# testing
#===============================================================================
if(SALGO_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif(SALGO_BUILD_TESTS)





#===============================================================================
# benchmarking
#===============================================================================
if(SALGO_BUILD_BENCHMARKS)
	#enable_testing()
	add_subdirectory(benchmark)
endif(SALGO_BUILD_BENCHMARKS)









#===============================================================================
# output build configuration
#===============================================================================
message("\n")
message("Build configuration:\n")
message("** Build type: " ${CMAKE_BUILD_TYPE})
message("** Build tests: " ${SALGO_BUILD_TESTS})
message("** Build benchmarks: " ${SALGO_BUILD_BENCHMARKS})
message("** Use libc++ (instead of default, usually libstdc++): " ${SALGO_USE_LIBCXX})
#message("** salgo version: " ${SALGO_VERSION})
#message("** Build shared libs: " ${SALGO_BUILD_SHARED})
#message("** Build docs: " ${SALGO_BUILD_DOC})
message("\n")

