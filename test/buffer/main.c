#include <assert.h>
#include <string.h>

#include "buffer.h"

static void test_insert_text(void)
{
    buffer *b = load_file_into_buffer("sample.txt");

    insert_text(b, 0, 0, "hello ", 6);

    size_t len;
    const char *line = buffer_get_line(b, 0, &len);

    assert(len == 11);
    assert(memcmp(line, "hello world", 11) == 0);

    free_buffer(b);
}

static void test_delete_text(void)
{
    buffer *b = load_file_into_buffer("sample.txt");

    delete_text(b, 0, 4, 1);

    size_t len;
    const char *line = buffer_get_line(b, 0, &len);

    assert(len == 4);
    assert(memcmp(line, "worl", 4) == 0);

    free_buffer(b);
}

int main(void)
{
    test_insert_text();
    test_delete_text();

    return 0;
}