add_executable(sena_editor_test
    main.c
    ../../src/editor/core/buffer.c
    ../../src/editor/core/editor.c
)

target_include_directories(sena_editor_test PRIVATE
    ../../include
)

add_test(
    NAME editor_tests
    COMMAND sena_editor_test
)

set_tests_properties(editor_tests PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)