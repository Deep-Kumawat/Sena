#include <editor.h>
#include <assert.h>
#include <string.h>

/*
* We will be loading multiple files into the buffer
*/
int main(void) {
    editor_state *editor = create_editor_state();
    if (!editor) {
        fprintf(stderr, "Error creating editor state\n");
        return 1;
    }

    // Add some files to the editor
    if (!add_buffer_to_editor(editor, "file.txt")) {
        fprintf(stderr, "Error adding file.txt to editor\n");
        return 1;
    }

    if (!add_buffer_to_editor(editor, "file1.txt")) {
        fprintf(stderr, "Error adding file1.txt to editor\n");
        return 1;
    }

    // get current buffer index
    size_t current_buffer_index = get_current_buffer_index(editor);

    assert(current_buffer_index == 1); // Should be 1 since we added two buffers and the current buffer index is 1

    // switch to the first buffer
    if (!switch_to_buffer(editor, 0)) {
        fprintf(stderr, "Error switching to buffer 0\n");
        return 1;
    }

    assert(get_current_buffer_index(editor) == 0); // Should be 0 since we switched to the first buffer

    // get the current cursor position in the active buffer
    position pos = get_current_buffer_position(editor);
    assert(pos.x == 0 && pos.y == 0); // Should be (0, 0) since we haven't moved the cursor yet

    // we will now move the cursor to a new position and add some text to the buffer
    pos.x = 4;
    pos.y = 0;
    set_current_buffer_position(editor, pos);

    assert(get_current_buffer_position(editor).x == 4 && get_current_buffer_position(editor).y == 0);

    // flush the current change in the current line to buffer   
    const char *text_before = get_current_line_from_buffer(editor, pos.y, NULL);
    const char *text_to_insert = "Hello";
    flush_current_line_change_to_buffer(editor, pos.y, pos.x, text_to_insert, strlen(text_to_insert)); 

    const char *line = get_current_line_from_buffer(editor, pos.y, NULL);

    assert(line != NULL);
    assert(strncmp(line, text_before, pos.x) == 0); // The text before the cursor should remain unchanged
    assert(strncmp(line + pos.x, text_to_insert, strlen(text_to_insert)) == 0); // The text after the cursor should be the inserted text

    // Clean up
    free_editor_state(editor);
    return 0;
}