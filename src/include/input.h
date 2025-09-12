#ifndef INPUT_H
#define INPUT_H

#include <stddef.h>   // for size_t
#include <sys/types.h> // for ssize_t

/*
 * InputBuffer is a simple wrapper for getline() input.
 * It holds the buffer, its allocated size, and the length
 * of the string actually read (excluding trailing newline).
 */
typedef struct {
    char*   buffer;
    size_t  buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer();
void read_input(InputBuffer* input_buffer);
void close_input_buffer(InputBuffer* input_buffer);

#endif // INPUT_H
