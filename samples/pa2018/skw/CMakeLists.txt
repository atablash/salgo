add_executable( salgo-sample-pa2018-skw skw.cpp )

add_custom_target( salgo-sample-pa2018-skw-copy-resources ALL
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test ${CMAKE_CURRENT_BINARY_DIR}/test || :
)

add_dependencies( salgo-sample-pa2018-skw
	salgo-samples-copy-resources
	salgo-sample-pa2018-skw-copy-resources
)

add_test( NAME salgo-sample-pa2018-skw COMMAND ../../resources/stdio-tester.py ./salgo-sample-pa2018-skw ./test )
