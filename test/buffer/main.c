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

static void test_insert_line(void)
{
    buffer *b = load_file_into_buffer("sample.txt");

    insert_line(b, 1);

    size_t len;
    const char *line = buffer_get_line(b, 1, &len);

    assert(len == 0 && line != NULL); // New line should be empty but not NULL

    free_buffer(b);
}

static void test_delete_line(void)
{
    buffer *b = load_file_into_buffer("sample.txt");

    delete_line(b, 0);

    size_t len;
    const char *line = buffer_get_line(b, 0, &len);

    assert(len == 0 && line == NULL); // After deleting the first line, the new first line should be empty

    free_buffer(b);
}

int main(void)
{
    test_insert_text();
    test_delete_text();
    test_insert_line();
    test_delete_line();
    return 0;
}