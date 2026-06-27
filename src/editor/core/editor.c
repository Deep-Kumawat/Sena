#include <string.h>
#include "editor.h"

struct buffer_state_s {
    char* filename; // Name of the file associated with the buffer
    buffer *buf; // Pointer to the buffer being edited
    position cursor; // Current cursor position in the buffer
};

struct editor_state_s {
    buffer_state *buffers[MAX_BUFFERS]; // Holds the pointers to the buffer states and not the buffers themselves
    size_t current_buffer_index; // Index of the currently active buffer
    bool buffer_active; // Flag to indicate if a buffer is currently active
};

editor_state* create_editor_state(void) {
    editor_state *state = (editor_state*)malloc(sizeof(editor_state));
    if (!state) {
        fprintf(stderr, "Error allocating memory for editor state\n");
        return NULL;
    }
    for (size_t i = 0; i < MAX_BUFFERS; ++i) {
        state->buffers[i] = NULL;
    }
    state->current_buffer_index = -1; // No buffer is active initially
    state->buffer_active = false; // use this flag to check to avoid referencing -1 index in case of empty
    return state;
}

bool add_buffer_to_editor(editor_state *editor_state, const char *filename) {
    if (!editor_state || !filename) {
        fprintf(stderr, "Error: Invalid editor state or filename\n");
        return false;
    }
    // Find the first available slot for a new buffer
    size_t index = 0;
    while (index < MAX_BUFFERS && editor_state->buffers[index] != NULL) {
        index++;
    }
    if (index == MAX_BUFFERS) {
        fprintf(stderr, "Error: Maximum number of buffers reached\n");
        return false;
    }

    // Load the file into a new buffer
    buffer *buf = load_file_into_buffer(filename);
    if (!buf) {
        fprintf(stderr, "Error: Failed to load file into buffer: %s\n", filename);
        return false;
    }

    // Create a new buffer state
    buffer_state *buf_state = (buffer_state*)malloc(sizeof(buffer_state));
    if (!buf_state) {
        fprintf(stderr, "Error allocating memory for buffer state\n");
        free_buffer(buf);
        return false;
    }
    buf_state->filename = strdup(filename); // Duplicate the filename string
    buf_state->buf = buf;
    buf_state->cursor.x = 0; // Initialize cursor position
    buf_state->cursor.y = 0;

    // Add the new buffer state to the editor
    editor_state->buffers[index] = buf_state;
    editor_state->current_buffer_index = index; // Set the current buffer to the newly added one
    editor_state->buffer_active = true; // Mark that a buffer is now active
    return true;
}

bool switch_to_buffer(editor_state *editor_state, size_t buffer_index) {
    if (!editor_state || buffer_index >= MAX_BUFFERS || !editor_state->buffers[buffer_index]) {
        fprintf(stderr, "Error: Invalid buffer index or editor state\n");
        return false;
    }
    editor_state->current_buffer_index = buffer_index;
    editor_state->buffer_active = true; // Mark that a buffer is now active
    return true;
}

size_t get_current_buffer_index(editor_state *editor_state) {
    if (!editor_state || !editor_state->buffer_active) {
        fprintf(stderr, "Error: No active buffer or invalid editor state\n");
        return (size_t)-1; // Return an invalid index
    }
    return editor_state->current_buffer_index;
}

void set_current_buffer_position(editor_state *editor_state, position pos) {
    if (!editor_state || !editor_state->buffer_active) {
        fprintf(stderr, "Error: No active buffer or invalid editor state\n");
        return;
    }
    size_t index = editor_state->current_buffer_index;
    if (index >= MAX_BUFFERS || !editor_state->buffers[index]) {
        fprintf(stderr, "Error: Invalid current buffer index\n");
        return;
    }
    // Normalize the position to ensure it's within the bounds of the buffer
    normalize_buffer_position(editor_state->buffers[index]->buf, &pos);
    editor_state->buffers[index]->cursor = pos;
}

position get_current_buffer_position(editor_state *editor_state) {
    if (!editor_state || !editor_state->buffer_active) {
        fprintf(stderr, "Error: No active buffer or invalid editor state\n");
        return (position){0, 0}; // Return a default position
    }
    size_t index = editor_state->current_buffer_index;
    if (index >= MAX_BUFFERS || !editor_state->buffers[index]) {
        fprintf(stderr, "Error: Invalid current buffer index\n");
        return (position){0, 0}; // Return a default position
    }
    return editor_state->buffers[index]->cursor;
}

void flush_current_line_change_to_buffer(editor_state *editor_state, size_t line_number, size_t column_number, const char *text, size_t length) {
    if (!editor_state || !editor_state->buffer_active) {
        fprintf(stderr, "Error: No active buffer or invalid editor state\n");
        return;
    }
    size_t index = editor_state->current_buffer_index;
    if (index >= MAX_BUFFERS || !editor_state->buffers[index]) {
        fprintf(stderr, "Error: Invalid current buffer index\n");
        return;
    }
    buffer *buf = editor_state->buffers[index]->buf;
    insert_text(buf, line_number, column_number, text, length);
}

void remove_current_line_change_from_buffer(editor_state *editor_state, size_t line_number, size_t column_number, size_t length) {
    if (!editor_state || !editor_state->buffer_active) {
        fprintf(stderr, "Error: No active buffer or invalid editor state\n");
        return;
    }
    size_t index = editor_state->current_buffer_index;
    if (index >= MAX_BUFFERS || !editor_state->buffers[index]) {
        fprintf(stderr, "Error: Invalid current buffer index\n");
        return;
    }
    buffer *buf = editor_state->buffers[index]->buf;
    delete_text(buf, line_number, column_number, length);
}

const char* get_current_line_from_buffer(editor_state *editor_state, size_t line_number, size_t *length) {
    if (!editor_state || !editor_state->buffer_active) {
        fprintf(stderr, "Error: No active buffer or invalid editor state\n");
        return NULL;
    }
    size_t index = editor_state->current_buffer_index;
    if (index >= MAX_BUFFERS || !editor_state->buffers[index]) {
        fprintf(stderr, "Error: Invalid current buffer index\n");
        return NULL;
    }
    buffer *buf = editor_state->buffers[index]->buf;
    return buffer_get_line(buf, line_number, length);
}

void remove_buffer_from_editor(editor_state *editor_state, size_t buffer_index) {
    if (!editor_state || buffer_index >= MAX_BUFFERS || !editor_state->buffers[buffer_index]) {
        fprintf(stderr, "Error: Invalid buffer index or editor state\n");
        return;
    }
    free_buffer(editor_state->buffers[buffer_index]->buf);
    free(editor_state->buffers[buffer_index]->filename);
    free(editor_state->buffers[buffer_index]);
    editor_state->buffers[buffer_index] = NULL;

    // If the removed buffer was the current one, reset the current buffer index
    if (editor_state->current_buffer_index == buffer_index) {
        editor_state->current_buffer_index = -1; // No active buffer
        editor_state->buffer_active = false;
    }
}

void save_and_free_buffer_from_editor(editor_state *editor_state, size_t buffer_index) {
    if (!editor_state || buffer_index >= MAX_BUFFERS || !editor_state->buffers[buffer_index]) {
        fprintf(stderr, "Error: Invalid buffer index or editor state\n");
        return;
    }

    buffer_state *buf_state = editor_state->buffers[buffer_index];
    // Save the buffer to disk
    save_buffer_to_file(buf_state->buf, buf_state->filename);

    // just free the buffer state and set the pointer to NULL
    free_buffer(buf_state->buf);
    free(buf_state->filename);
    free(buf_state);
    editor_state->buffers[buffer_index] = NULL;
}

void free_editor_state(editor_state *editor_state) {
    if (!editor_state) {
        return;
    }
    for (size_t i = 0; i < MAX_BUFFERS; ++i) {
        if (editor_state->buffers[i]) {
            free_buffer(editor_state->buffers[i]->buf);
            free(editor_state->buffers[i]->filename);
            free(editor_state->buffers[i]);
        }
    }
    free(editor_state);
}