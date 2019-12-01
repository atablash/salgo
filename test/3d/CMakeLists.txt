add_executable( salgo-unit-tests-3d
    resources.cpp

    compute-normals.cpp
    cap-holes.cpp
    collapse-edges.cpp

    ../../third-party/tinyply.cpp

    ../main.cpp
)


add_custom_target( copy-resources ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources || :
)

add_dependencies( salgo-unit-tests-3d
    copy-resources
)

add_test( salgo-unit-tests-3d salgo-unit-tests-3d )
