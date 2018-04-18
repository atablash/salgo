

find_package(Benchmark REQUIRED)
if (Benchmark_FOUND)
	include_directories( ${Benchmark_INCLUDE_DIRS} )
	link_libraries( ${Benchmark_LIBRARIES} )

	message("Benchmark_INCLUDE_DIRS: " ${Benchmark_INCLUDE_DIRS})
	message("Benchmark_LIBRARIES: " ${Benchmark_LIBRARIES})

	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	link_libraries( ${CMAKE_THREAD_LIBS_INIT} ) # required by GoogleTest / Benchmark
endif()




link_libraries( ${SALGO_CXX_LINKER_FLAGS} )



# make #include <salgo/...> work
include_directories( # SYSTEM
	../include
)


add_executable( allocator  allocator.cpp )
add_executable(	hash-table hash-table.cpp )
add_executable(	list       list.cpp )
add_executable(	vector     vector.cpp )


