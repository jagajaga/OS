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

void _read(int fd, char * buffer, size_t size)
{

}

int main (int argc, char ** argv)
{
    int k = get_int(argv[1]);
	char * buffer;
	buffer = malloc(k);
	read(0, buffer, k);
	write(1, buffer, k);
	write(1, buffer, k);
    return 0;
}
