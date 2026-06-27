#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "position.h"

/* Forward declaration of the Buffer structure, each text file owns a buffer */
typedef struct buffer_s buffer;

/* 
* Allocates a new buffer and loads the contents of the specified file into it 
*/
buffer* load_file_into_buffer(const char* filename);

/*
* Text edits
*/

/*
* Inserts the specified text at the given line and column in the buffer
*/
void insert_text(buffer *buffer, size_t line_number, size_t column_number, const char *text, size_t length);

/*
* Deletes the specified number of characters starting from the given line and column in the buffer
*/
void delete_text(buffer *buffer, size_t line_number, size_t column_number, size_t length);

/*
* Inserts a new line at the specified line number in the buffer
*/
void insert_line(buffer *buffer, size_t line_number);

/*
* Deletes the line at the specified line number in the buffer
*/
void delete_line(buffer *buffer, size_t line_number);

/*
* Returns the number of lines in the buffer
*/
size_t buffer_line_count(buffer *buffer);

/*
* Returns a pointer to the line at the specified line number in the buffer
*/  
const char *buffer_get_line(buffer *buffer, size_t line_number, size_t *length);

/*
* Check if the position is valid for the given buffer, i.e. if the line and column numbers are within the bounds of the buffer
* If the position is invalid, it will be adjusted to the nearest valid position within the buffer
*/
void normalize_buffer_position(buffer *buffer, position *pos);

/*
* Save the buffer to the specified file, overwriting the existing contents of the file
*/
void save_buffer_to_file(buffer *buffer, const char *filename);

/* 
* Frees the memory allocated for the buffer
*/
void free_buffer(buffer *buffer);

#endif // BUFFER_HS