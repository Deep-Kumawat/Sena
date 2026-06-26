#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "position.h"

#define MAX_BUFFERS 256 // I wont be opening more than 256 files for sure

/*
* The editor should be able to handle multiple buffers one for each file
* State for each buffer should be maintained separately, including the cursor position, selection, etc.
*/

/*
* This structure holds the state of the editor for a specific buffer, 
* including the cursor position and the current buffer being edited.
*/
typedef struct buffer_state_s buffer_state;

/*
* This structure holds the overall state of the editor, 
* including the list of buffers and the current buffer being edited.
*/
typedef struct editor_state_s editor_state;

/*
* Function to create a new editor state.
*/
editor_state* create_editor_state(void);

/*
* Function to add a new buffer to the editor.
*/
bool add_buffer_to_editor(editor_state *editor_state, const char *filename);

/*
* Function to switch to a different buffer.
*/
bool switch_to_buffer(editor_state *editor_state, size_t buffer_index);

/*
* Function to get the index of the current buffer.
*/
size_t get_current_buffer_index(editor_state *editor_state);

/*
* Function to get the current cursor position in the active buffer.
*/
position get_current_buffer_position(editor_state *editor_state);

/**
* Function to set the current cursor position in the active buffer.
* TODO: this would be slow since we are passing the position by value
* Rather call individual set_cursor_x and set_cursor_y functions to avoid copying the struct
*/
void set_current_buffer_position(editor_state *editor_state, position pos);

/*
* function to flush the current change in the current line to buffer
* requires the line number and column from which the change is added
*/
void flush_current_line_change_to_buffer(editor_state *editor_state, size_t line_number, size_t column, const char *text, size_t length);

/*
* function to remove the current change in the current line from buffer
*/
void remove_current_line_change_from_buffer(editor_state *editor_state, size_t line_number, size_t column, size_t length);

/*
* function to return the line, this is useful just for testing interface, we will not be using this in the editor
*/
const char* get_current_line_from_buffer(editor_state *editor_state, size_t line_number, size_t *length);

/*
* function to remove the buffer
*/
void remove_buffer_from_editor(editor_state *editor_state, size_t buffer_index);

/*
* Function to free the memory allocated for the editor state.
*/
void free_editor_state(editor_state *editor_state);

#endif // EDITOR_H