
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
	inplace-storage.cpp

	named-arguments.cpp

	crude-allocator.cpp

	memory-block.cpp
	dynamic-array.cpp
	sparse-array.cpp
	chunked-array.cpp
	unordered-array.cpp

	hash-table.cpp

	list.cpp

	graph.cpp
	binary-forest.cpp
	union-find.cpp

	modulo.cpp
	binomial.cpp
	miller-rabin.cpp

	kd.cpp
)


# make #include <salgo/...> work
include_directories( # SYSTEM
	../include
)

include_directories( # SYSTEM
	../third-party
)




link_libraries( ${SALGO_CXX_LINKER_FLAGS} )



add_executable(	salgo-unit-tests ${sources} )
add_test( salgo-unit-tests salgo-unit-tests )


add_subdirectory( g3d )
