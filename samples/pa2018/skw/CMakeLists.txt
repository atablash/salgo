add_executable( salgo-sample-pa2018-skw skw.cpp )

add_dependencies( salgo-sample-pa2018-skw
	salgo-samples-copy-resources
)

add_test( NAME salgo-sample-pa2018-skw COMMAND ../../resources/stdio-tester.py ./skw ./test )
