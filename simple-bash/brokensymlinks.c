#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>


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

void brokensymlinks(char * name)
{
    DIR * d;
    struct dirent * dir;
    struct stat sbuf;
    if (name == ".")
    {
        return;
    }
    d = opendir(name);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
            lstat(dir->d_name, &sbuf);
            if (S_ISDIR(sbuf.st_mode))
            {
                brokensymlinks(dir->d_name);
            }
            if (access(dir->d_name, F_OK) && S_ISLNK(sbuf.st_mode))
            {
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
}

int main(int argc, char ** argv)
{
    char * path = argv[1];
    printf("%s", path);
    brokensymlinks(path);
    return 0;
}
