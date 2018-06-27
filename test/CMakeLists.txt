
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



link_libraries( ${SALGO_CXX_LINKER_FLAGS} )



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
	#splay-tree.cpp
	#3d/compute-normals.cpp
	#3d/collapse-edges.cpp
	#3d/cap-holes.cpp
	#3d/links.cpp
	#3d/const.cpp
	#../third-party/tinyply.cpp
)


# make #include <salgo/...> work
include_directories( # SYSTEM
	../include
)

include_directories( # SYSTEM
	../third-party
)


add_executable(	all-tests ${sources} )
add_test( all-tests all-tests )


