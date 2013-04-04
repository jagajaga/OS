#include <unistd.h>
#include <stdlib.h>

int get_int(char * c)
{
    int result = 0;
    int i = 0;
    for (i = 0; c[i] != 0; i++)
    {
        result = (result * 10) + (int) (c[i] - '0');
    }
    return result;
}

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
size_t _read(int fd, char * buffer, size_t size)
{
    size_t current = 0;
    while (current < size)
    {
        size_t result = read(fd, buffer + current, size);
        if (result < 1)
        {
            return current;
        }
        current = current + result;
        size -= current;
    }
    return current;
}

int main (int argc, char ** argv)
{
    int k = get_int(argv[1]);
    char * buffer  = malloc(k + 1);
    char c;
    size_t i, trim_flag = 0, count = 0, buffer_result = 0, flag = 1;
    char * new_line = "\n";
    while(1)
    {
        buffer_result = _read(0, buffer + count, k + 1 - count);
        if (buffer_result < 1)
        {
            break;
        }
        /* while (buffer[0] == '\n') */
        /* { */
        /*     buffer_result--; */
        /*     memmove(buffer, buffer + 1, buffer_result); */
        /*     trim_flag = 1; */
        /* } */
        /* if (trim_flag) */
        /* { */
        /*     trim_flag = 0; */
        /*     if (buffer_result == 0) */
        /*     { */
        /*         count = 0; */
        /*         continue; */
        /*     } */
        /*     count =  k + 1 - buffer_result; */
        /*     continue; */
        /* } */
        buffer_result += count;
        while (count < buffer_result)
        {
            c = buffer[count];
            if (c == '\n')
            {
                if (flag)
                {
                    _write(1, buffer, count);
                    _write(1, new_line, 1);
                    _write(1, buffer, count);
                    _write(1, new_line, 1);
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
        if (count == k + 1)
        {
            flag = 0;
            count = 0;
        }
    }
    if (count <= k && flag && count)
    {
        _write(1, buffer, count);
        _write(1, new_line, 1);
        _write(1, buffer, count);
        _write(1, new_line, 1);
    }
    free(buffer);
    return 0;
}
