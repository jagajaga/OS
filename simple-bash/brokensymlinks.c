#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

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
    struct dirent * file;
    struct stat sbuf;
    if (name == ".")
    {
        return;
    }
    d = opendir(name);
    size_t len = strlen(name);
    char * buf = malloc(len + 300);
    memcpy(buf, name, sizeof(char) * (len + 1));
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
            memcpy(buf + len, file->d_name, sizeof(char) * (strlen(file->d_name) + 1));
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
    char * path = argv[1];
    brokensymlinks(path);
    return 0;
}
