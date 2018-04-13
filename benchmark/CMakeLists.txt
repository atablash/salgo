
#===============================================================================
# Additional cmake modules
#===============================================================================
#set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
#message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})





find_package(Benchmark REQUIRED)
if (Benchmark_FOUND)
	include_directories( ${Benchmark_INCLUDE_DIRS} )
	link_libraries( ${Benchmark_LIBRARIES} )

	#message("Benchmark_INCLUDE_DIRS: " ${Benchmark_INCLUDE_DIRS})
	#message("Benchmark_LIBRARIES: " ${Benchmark_LIBRARIES})

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	link_libraries( ${CMAKE_THREAD_LIBS_INIT} ) # required by GoogleTest / Benchmark
endif()




#find_package(GTest REQUIRED)
#if (GTEST_FOUND)
#	include_directories( ${GTEST_INCLUDE_DIRS} )
#	link_libraries( ${GTEST_LIBRARIES} )
#	# link_libraries( ${GTEST_MAIN_LIBRARIES} )
#
#	# required by GTest
#	set(THREADS_PREFER_PTHREAD_FLAG ON)
#	find_package(Threads REQUIRED)
#endif()






#find_package(gflags REQUIRED)
#if (GFLAGS_FOUND)
#    include_directories(${GFLAGS_INCLUDE_DIRS})
#    link_libraries(${GFLAGS_LIBRARIES})
#endif()



# make #include <salgo/...> work
include_directories( # SYSTEM
	../include
)

#include_directories( SYSTEM
#	../third-party
#)


add_executable(	hash-table hash-table.cpp )


# cotire(all-tests)

# add_test( hash-table hash-table )


