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
    size_t current;
    while (current <= size)
    {
        size_t result = write(fd, buf +current, size);
        if (result == -1)
        {
            break;
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
    size_t current;
    while(current <= size)
    {
        size_t result = read(fd, buffer + current, size);
        if (result == -1)
        {
            break;
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
    char * buffer;
    buffer = malloc(k);
    _read(0, buffer, k);
    _write(1, buffer, k);
    return 0;
}
