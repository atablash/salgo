add_executable( skw skw.cpp )
add_test( NAME pa2018-skw COMMAND ../../stdio-tester.py ./skw ./test )
