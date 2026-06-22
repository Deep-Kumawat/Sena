add_executable(sena_buffer_test
    main.c
    ../../src/editor/core/buffer.c
)

target_include_directories(sena_buffer_test PRIVATE
    ../../include
)

add_test(
    NAME buffer_tests
    COMMAND sena_buffer_test
)

set_tests_properties(buffer_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)