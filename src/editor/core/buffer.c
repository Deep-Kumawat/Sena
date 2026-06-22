#include "buffer.h"

/*
* @file buffer.c
* @brief Implementation of the buffer management for the text editor.
*
* The buffer is a contiguous block of memory which holds the content of the
* file. A line table is used to track the current lines in the document.
*
* The line table contains descriptors which reference line content stored
* in the buffer. Text storage is append only; modified lines are written
* to newly allocated space and old versions remain in the buffer.
*
* INSERT IN A LINE:
* When characters are inserted into a line, new space is allocated at the
* end of the buffer. The part before the insertion point is copied to the
* new space, followed by the inserted text, followed by the remaining part
* of the line. The line descriptor is then updated to reference the new
* version of the line. The old version is not removed from the buffer.
*
* DELETE IN A LINE:
* When characters are deleted from a line, we will remove the character
* from the line and move the remaining characters to fill the gap.
*
* DELETE A LINE:
* When a line is deleted, its line descriptor is removed from the line
* table. The line content itself is not removed from the buffer.
*
* INSERT A LINE:
* When a line is inserted, storage is allocated for the new line, a new
* line descriptor is created, and the descriptor is inserted into the
* line table.
*
* TODO: This is really bad, will lead to fragmentation and will suck for 
* large files. Need to implement a better approach later, for now this 
* should be just fine for small files, I dont see this blowing up until 
* 100k lines or longer lines of text, but we will see.
*/

/*
* There is line table which will be used to track where the lines are in the buffer.
*/
struct line_entry_s {
    size_t offset; // Offset of the line in the buffer
    size_t length; // Length of the line
};

typedef struct line_entry_s line_entry_t;

typedef struct {
    line_entry_t *entries; // Array of line entries
    size_t count; // Number of lines
} line_table_t;

/*
* The buffer data is a contiguous block of memory which holds the content of the file.
* This holds the actual text data and the line table as well.
*/
struct buffer_s {
    char* data; // Pointer to the buffer data, will be allocated dynamically
    size_t size; // Size of the buffer
    line_table_t line_table; // The line table for tracking lines in the buffer
};

/* 
* This allocates the buffer, on failure this method will free up any resources it has allocated and return NULL, 
* on success it will return a pointer to the buffer.
*/
buffer* load_file_into_buffer(const char* filename) {
    FILE *f = fopen(filename, "r");
    long file_size;
    char *file_contents;
    size_t bytes_read, line_count = 0, line_index = 0;

    if (!f) {
        // errno is set by fopen on any error
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }
    // We have to do the following things once the file is open
    // 1. Get the size of the file, if empty we will return an empty buffer
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // 2. Allocate a buffer of the appropriate size
    file_contents = (char*)malloc(file_size + 1); // +1 for null terminator
    if (!file_contents) {
        fprintf(stderr, "Error allocating buffer for file: %s\n", filename);
        fclose(f);
        return NULL;
    }
    file_contents[file_size] = '\0'; // Null terminate the buffer, not strictly necessary

    // 3. Copy the file contents into the buffer
    bytes_read = fread(file_contents, sizeof(char), file_size, f);

    if (bytes_read == file_size) {
        // Successfully read the entire file
        fclose(f);
    }
    else {
        // error handling
        if (feof(f)) {
            fprintf(stderr, "Error reading file: %s\n", filename);
        }
        else if (ferror(f)) {
            fprintf(stderr, "Error reading file: %s\n", filename);
        }
        free(file_contents);
        fclose(f);
        return NULL;
    }

    // 4. Count the number of lines in the file
    for (size_t i = 0; i < bytes_read; ++i) {
        if (file_contents[i] == '\n') {
            line_count++;
        }
    }

    if (bytes_read > 0 && file_contents[bytes_read - 1] != '\n') {
        line_count++; // Account for the last line if it doesn't end with a newline
    }

    // 5. Initialize the line table
    line_entry_t *line_entries = (line_entry_t*)malloc(line_count * sizeof(line_entry_t));
    if (!line_entries) {
        fprintf(stderr, "Error allocating line table for file: %s\n", filename);
        free(file_contents);
        return NULL;
    }

    // 6. Populate the line table with offsets and lengths
    line_count = 0;

    for (size_t i = 0; i < bytes_read; ++i) {
        if (file_contents[i] == '\n') {
            line_entries[line_count].offset = line_index;
            line_entries[line_count].length = i - line_index;
            line_index = i + 1; // Move to the start of the next line
            line_count++;
        }
    }

    if (line_index < bytes_read) {
        // Handle the last line if it doesn't end with a newline
        line_entries[line_count].offset = line_index;
        line_entries[line_count].length = bytes_read - line_index;
        line_count++;
    }

    // Initialize the buffer structure, if successful
    buffer* b = (buffer*)malloc(sizeof(buffer));
    if (!b) {
        fprintf(stderr, "Error allocating buffer structure for file: %s\n", filename);
        free(file_contents);
        free(line_entries);
        return NULL;
    }
    b->size = bytes_read;
    b->data = file_contents;
    b->line_table.entries = line_entries;
    b->line_table.count = line_count;
    return b;
}

/*
* Should be called by the caller when they are done with the buffer
*/
void free_buffer(buffer *buffer) {
    // Implementation to free the buffer
    if (buffer) {
        free(buffer->data);
        free(buffer->line_table.entries);
        free(buffer);
    }
}