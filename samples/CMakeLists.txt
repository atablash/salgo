include_directories(
	../include
)

link_libraries( ${SALGO_CXX_LINKER_FLAGS} )


add_custom_target( salgo-samples-copy-resources ALL
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources || :
)


add_subdirectory( pa2018 )
