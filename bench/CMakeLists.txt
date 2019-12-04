

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


add_executable( salgo-bench-allocator  allocator.cpp )
add_test( salgo-bench-allocator salgo-bench-allocator )

add_executable(	salgo-bench-hashtable hash-table.cpp )
add_test( salgo-bench-hashtable salgo-bench-hashtable )

add_executable(	salgo-bench-list       list.cpp )
add_test( salgo-bench-list salgo-bench-list )

add_executable(	salgo-bench-dynamic-array   dynamic-array.cpp )
add_test( salgo-bench-dynamic-array salgo-bench-dynamic-array )


