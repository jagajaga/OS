#include <unistd.h>
#include <stdio.h>

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

int main (int argc, char ** argv)
{
    int k = get_int(argv[1]); 
	write(1, k, );
    return 0;
}
