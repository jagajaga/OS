#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

void brokensymlinks(char * name)
{
    DIR * d;
    struct dirent * file;
    const static char point = '.';

    if (name == &point)
    {
        return;
    }

    d = opendir(name);
    size_t len = strlen(name);
    const int buffer_length = len + NAME_MAX + 1;

    if (buffer_length > PATH_MAX)
    {
        const static char * long_name = "\nThis name is too long.";
        write(2, name, strlen(name));
        write(2, long_name, strlen(long_name));
        return;
    }

    char * buf = malloc(buffer_length);

    if (buf == NULL)
    {
        _exit(2);
    }

    memcpy(buf, name, len + 1);

    if (buf[len - 1] != '/')
    {
        buf[len] = '/';
        len++;
        buf[len] = 0;
    }

    if (d)
    {
        while ((file = readdir(d)) != NULL)
        {
            if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
            {
                continue;
            }

            memcpy(buf + len, file->d_name, strlen(file->d_name) + 1);

            if (file->d_type == DT_DIR)
            {
                brokensymlinks(buf);
            }

            if ((access(buf, F_OK) == -1) && (file->d_type == DT_LNK))
            {
                printf("%s\n", buf);
            }

            buf[len] = 0;
        }

        closedir(d);
    }

    free(buf);
}

int main(int argc, char ** argv)
{
    char * path;
    int i;

    for (i = 1; i < argc; i++)
    {
        path = argv[i];
        brokensymlinks(path);
    }

    return 0;
}
