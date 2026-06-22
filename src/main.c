#include <stdio.h>
#include "buffer.h"

int main (int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    buffer *b;
    b = load_file_into_buffer(argv[1]);
    if (!b) {
        fprintf(stderr, "Failed to load file into buffer: %s\n", argv[1]);
        return 1;
    }
    free_buffer(b);
    return 0;
}