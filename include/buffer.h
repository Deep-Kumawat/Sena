#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Forward declaration of the Buffer structure, each text file owns a buffer */
typedef struct buffer_s buffer;

/* 
* Allocates a new buffer and loads the contents of the specified file into it 
*/
buffer* load_file_into_buffer(const char* filename); 

/* 
* Frees the memory allocated for the buffer
*/
void free_buffer(buffer *buffer);

#endif // BUFFER_HS