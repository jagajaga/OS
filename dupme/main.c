#include <unistd.h>
#include <stdlib.h>
#include <string.h>

size_t _write(int fd, char * buffer, size_t size)
{
    size_t current = 0;

    while (current < size)
    {
        size_t result = write(fd, buffer + current, size);

        if (result < 1)
        {
            return current;
        }

        current += result;
        size -= result;
    }

    return current;
}


void write_buffer_quad(int fd, char * buffer, size_t buffer_size, char * separator, size_t separator_size)
{
    _write(fd, buffer, buffer_size);
    _write(fd, separator, separator_size);
    _write(fd, buffer, buffer_size);
    _write(fd, separator, separator_size);
}


int main (int argc, char ** argv)
{
    if (argc < 2)
    {
        _write(1, "Wrong arguments size", 22);
        return 1;
    }

    int k = atoi(argv[1]);
    char * buffer  = malloc(k + 1);
    char c;
    size_t count = 0, buffer_result = 0, flag = 1;
    char * new_line = "\n";

    while(1)
    {
        buffer_result = read(0, buffer + count, k + 1 - count);

        if (buffer_result < 1)
        {
            break;
        }

        buffer_result += count;

        while (count < buffer_result)
        {
            c = buffer[count];

            if (c == '\n')
            {
                if (flag)
                {
                    write_buffer_quad(1, buffer, count, new_line, 1);
                }
                else
                {
                    flag = 1;
                }

                ++count;
                buffer_result -= count;
                memmove(buffer, buffer + count, buffer_result);
                count = 0;
            }
            else
            {
                ++count;
            }
        }

        if (count == (unsigned) k + 1)
        {
            flag = 0;
            count = 0;
        }
    }

    if (count <= (unsigned) k && flag && count)
    {
        write_buffer_quad(1, buffer, count, new_line, 1);
    }

    free(buffer);
    return 0;
}
