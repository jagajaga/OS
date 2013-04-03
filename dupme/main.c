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
    while (current <= size)
    {
        size_t result = write(fd, buffer + current, size);
        if (result < 1)
        {
            return result;
        }
        current += result;
        size -= result;
        if (size == 0)
        {
            return  current;
        }
    }
    return -1;
}

size_t _read(int fd, char * buffer, size_t size)
{
    size_t current = 0;
    while(current <= size)
    {
        size_t result = read(fd, buffer + current, size);
        if (result < 1)
        {
            return result;
        }
        current += result;
        size -= result;
        if (size == 0)
        {
            return current;
        }
    }
    return -1;
}

int main (int argc, char ** argv)
{
    int k = get_int(argv[1]);
    char * buffer  = malloc(k + 1);
    char c;
    size_t i, c_result = -1, count = 0, buffer_result = 0, flag = 1;
    char * new_line = "\n";
    while((buffer_result += _read(0, buffer + count, k + 1 - count)) > 0)
    {
        count = 0;
        while (buffer[0] == '\n')
        {
            memmove(buffer, buffer + 1, buffer_result - 1);
            buffer_result--;
        }
        if (buffer_result < k + 1)
        {
            count =  k + 1 - buffer_result;
            continue;
        }
        while(count < buffer_result)
        {
            while (c != '\n' && count < buffer_result)
            {
                c = buffer[count];
                if (c == '\n')
                {
                    break;
                }
                count++;
            }
            if (count <= k)
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
                count++;
                memmove(buffer, buffer + count, buffer_result - count);
		buffer_result -= count;
                count = 0;
            }
            else
            {
                flag = 0;
            }
        }
    }
    free(buffer);
    return 0;
}
