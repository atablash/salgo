add_executable( unit-tests-3d
    compute-normals.cpp
    cap-holes.cpp
    collapse-edges.cpp

    ../../third-party/tinyply.cpp

    ../main.cpp
)

add_test( unit-tests-3d unit-tests-3d )
