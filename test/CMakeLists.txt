
#===============================================================================
# Additional cmake modules
#===============================================================================
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})




find_package(GTest REQUIRED)
if (GTEST_FOUND)
	include_directories( ${GTEST_INCLUDE_DIRS} )
    link_libraries( ${GTEST_LIBRARIES} )
	# link_libraries( ${GTEST_MAIN_LIBRARIES} )

	# required by GTest
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
endif()






find_package(gflags REQUIRED)
if (GFLAGS_FOUND)
    include_directories(${GFLAGS_INCLUDE_DIRS})
    link_libraries(${GFLAGS_LIBRARIES})
endif()


set(sources
	main.cpp
	sparse-vector.cpp
)


# make #include <salgo/...> work
include_directories( # SYSTEM
	../include
)

include_directories( SYSTEM
	../third-party
)


add_executable(	all-tests ${sources} )

target_link_libraries( all-tests
	${CMAKE_THREAD_LIBS_INIT} # required by GoogleTest
)

# cotire(all-tests)

add_test( all-tests all-tests )


