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

size_t _write(int fd, char * buf, size_t size)
{
    size_t current = 0;
    while (current <= size)
    {
        size_t result = write(fd, buf +current, size);
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
    char * buffer  = malloc(k);
    char c;
    size_t i, c_result = -1, count = 0;
    char * new_line = "\n";
    while (c_result != 0)
    {
        c_result = _read(0, &c, 1);
        while((c != '\n') && (c_result != 0) && (count <= k))
        {
            buffer[count] = c;
            count++;
            c_result = _read(0, &c, 1);
        }
        if (count <= k)
        {
            if (buffer[0] != '\n' && count)
            {
                _write(1, buffer, count);
                _write(1, new_line, 1);
                _write(1, buffer, count);
                _write(1, new_line, 1);
            }
            count = 0;
            continue;
        }
        count = 0;
        c_result = -1;
        while ((c != '\n'))
        {
            c_result = _read(0, &c, 1);
            if(c_result == 0)
            {
		free(buffer);
                return 0;
            }
        }
    }
    free(buffer);
    return 0;
}
