#include <string.h>
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

#define INITIAL_BUFFER_SIZE 1024

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
    size_t capacity; // Capacity of the line table
} line_table_t;

/*
* The buffer data is a contiguous block of memory which holds the content of the file.
* This holds the actual text data and the line table as well.
*/
struct buffer_s {
    char* data; // Pointer to the buffer data, will be allocated dynamically
    size_t size; // Size of the buffer
    size_t used; // Amount of the buffer currently used
    line_table_t line_table; // The line table for tracking lines in the buffer
};

static bool reallocate_buffer(buffer *buffer, size_t new_size) {
    // Implementation to reallocate the buffer to a new size
    char *new_data = (char*)realloc(buffer->data, new_size);
    if (!new_data) {
        fprintf(stderr, "Error reallocating buffer to new size: %zu\n", new_size);
        return false; // Reallocation failed
    }
    buffer->data = new_data;
    buffer->size = new_size;
    return true; // Reallocation successful
}

static bool reallocate_line_table(buffer *buffer, size_t new_capacity) {
    // Implementation to reallocate the line table to a new capacity
    line_entry_t *new_entries = (line_entry_t*)realloc(buffer->line_table.entries, new_capacity * sizeof(line_entry_t));
    if (!new_entries) {
        fprintf(stderr, "Error reallocating line table to new capacity: %zu\n", new_capacity);
        return false; // Reallocation failed
    }
    buffer->line_table.entries = new_entries;
    buffer->line_table.capacity = new_capacity;
    return true; // Reallocation successful
}

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
    file_contents = (char*)malloc(file_size + 1 + INITIAL_BUFFER_SIZE); // +1 for null terminator and initial buffer size for future edits
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
            line_entries[line_count].length = i - line_index; // Newline is not included in the length
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
    b->size = bytes_read + 1 + INITIAL_BUFFER_SIZE; // +1 for null terminator and initial buffer size for future edits
    b->used = bytes_read;
    b->data = file_contents;
    b->line_table.entries = line_entries;
    b->line_table.count = line_count;
    b->line_table.capacity = line_count;
    return b;
}

void insert_text(buffer *buffer, size_t line_number, size_t column_number, const char *text, size_t length) {
    // Implementation to insert text into the buffer at the specified line and column
    size_t line_length;
    char *dest, *src;
    if (line_number >= buffer->line_table.count) {
        fprintf(stderr, "Error: Line number %zu is out of bounds\n", line_number);
        return;
    }
    if (!text) {
        fprintf(stderr, "Error: Text to insert is NULL\n");
        return;
    }

    line_length = buffer->line_table.entries[line_number].length;

    if (column_number > line_length) {
        fprintf(stderr, "Error: Column number %zu is out of bounds for line %zu\n", column_number, line_number);
        return;
    }

    // Check till where the buffer is currently used, if the new text exceeds the buffer size 
    // we will need to reallocate the buffer with more space
    if (buffer->used + line_length + length > buffer->size) {
        // Reallocate the buffer with more space
        size_t new_size = buffer->size * 2 + length; // Double the size
        if (!reallocate_buffer(buffer, new_size)) {
            fprintf(stderr, "Error inserting text\n");
            return;
        }
    }

    // using memcpy is faster here ( no overlap )
    // copy the part before the insertion point to the new location
    dest = buffer->data + buffer->used;
    src = buffer->data + buffer->line_table.entries[line_number].offset;
    memcpy(dest, src, column_number);
    // copy the new text to the new location
    dest = buffer->data + buffer->used + column_number;
    src = (char*)text;
    memcpy(dest, src, length);
    // no need to copy the part after insertion point, if we insert at end
    if (column_number == line_length) {
        // update the line descriptor to point to the new version of the line
        buffer->line_table.entries[line_number].offset = buffer->used;
        buffer->line_table.entries[line_number].length = line_length + length;
        buffer->used += line_length + length; // Update the used size of the buffer
        return;
    }
    // copy the part after the insertion point to the new location
    dest = buffer->data + buffer->used + column_number + length;
    src = buffer->data + buffer->line_table.entries[line_number].offset + column_number;
    memcpy(dest, src, line_length - column_number);

    // update the line descriptor to point to the new version of the line
    buffer->line_table.entries[line_number].offset = buffer->used;
    buffer->line_table.entries[line_number].length = line_length + length;
    buffer->used += line_length + length; // Update the used size of the buffer
}

void delete_text(buffer *buffer, size_t line_number, size_t column_number, size_t length) {
    // Implementation to delete text from the buffer at the specified line and column
    size_t line_length;
    char *dest, *src;

    // memove is used here because there is overlap when we are moving the remaining text to fill the gap
    if (line_number >= buffer->line_table.count) {
        fprintf(stderr, "Error: Line number %zu is out of bounds\n", line_number);
        return;
    }

    line_length = buffer->line_table.entries[line_number].length;

    if (column_number >= line_length) {
        fprintf(stderr, "Error: Column number %zu is out of bounds for line %zu\n", column_number, line_number);
        return;
    }

    if (column_number + length > line_length) {
        length = line_length - column_number; // Adjust length to delete only till the end of the line
    }

    // if delete from the middle, we have to move
    // else if deleting from the end, we just need to update the line length
    if (column_number + length < line_length) {
        // Move the remaining text to fill the gap
        dest = buffer->data + buffer->line_table.entries[line_number].offset + column_number;
        src = buffer->data + buffer->line_table.entries[line_number].offset + column_number + length;
        memmove(dest, src, line_length - column_number - length);
    }
    
    // Update the line descriptor to reflect the new length of the line
    buffer->line_table.entries[line_number].length = line_length - length;
    // no need to update the used size of the buffer, 
    // as we are not actually removing any data from the buffer, just updating the line descriptor
}

void insert_line(buffer *buffer, size_t line_number) {
    line_entry_t new_line_entry;
    // Implementation to insert a new line into the buffer at the specified line number
    if (line_number > buffer->line_table.count) {
        fprintf(stderr, "Error: Line number %zu is out of bounds\n", line_number);
        return;
    }

    // Allocate space for the new line in the buffer
    if (buffer->used + 1 > buffer->size) {
        // Reallocate the buffer with more space
        size_t new_size = buffer->size * 2 + 1; // Double the size
        if (!reallocate_buffer(buffer, new_size)) {
            fprintf(stderr, "Error inserting line\n");
            return;
        }
    }

    // Check if we need to reallocate the line table to accommodate the new line
    if (buffer->line_table.count + 1 > buffer->line_table.capacity) {
        size_t new_capacity = buffer->line_table.capacity * 2 + 1; // Double the capacity
        if (!reallocate_line_table(buffer, new_capacity)) {
            fprintf(stderr, "Error reallocating line table\n");
            return;
        }
    }

    // create a new line entry for the new line
    new_line_entry.offset = buffer->used; // New line starts at the end of the used buffer
    new_line_entry.length = 0; // New line is empty initially

    // Insert the new line entry into the line table
    if (line_number < buffer->line_table.count) {
        // Shift existing line entries down to make space for the new line
        memmove(&buffer->line_table.entries[line_number + 1],
                &buffer->line_table.entries[line_number],
                (buffer->line_table.count - line_number) * sizeof(line_entry_t));
    }
    buffer->line_table.entries[line_number] = new_line_entry;
    buffer->line_table.count++;
}

void delete_line(buffer *buffer, size_t line_number) {
    // Implementation to delete a line from the buffer at the specified line number
    if (line_number >= buffer->line_table.count) {
        fprintf(stderr, "Error: Line number %zu is out of bounds\n", line_number);
        return;
    }
    // Shift existing line entries up to fill the gap left by the deleted line
    memmove(&buffer->line_table.entries[line_number],
            &buffer->line_table.entries[line_number + 1],
            (buffer->line_table.count - line_number - 1) * sizeof(line_entry_t));
    buffer->line_table.count--;
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