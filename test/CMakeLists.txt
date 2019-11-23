
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
	link_libraries( ${CMAKE_THREAD_LIBS_INIT} ) # required by GoogleTest
endif()





set(sources
	main.cpp
	stack-storage.cpp
	memory-block.cpp
	vector.cpp
	vector-sparse.cpp
	crude-allocator.cpp
	list.cpp
	unordered-vector.cpp
	hash-table.cpp
	chunked-vector.cpp
	rooted-forest.cpp
	graph.cpp
	modulo.cpp
	binomial.cpp
	union-find.cpp

	miller-rabin.cpp
)


# make #include <salgo/...> work
include_directories( # SYSTEM
	../include
)

include_directories( # SYSTEM
	../third-party
)






add_executable(	unit-tests ${sources} )
add_test( unit-tests unit-tests )

link_libraries( ${SALGO_CXX_LINKER_FLAGS} )

add_subdirectory( 3d )
