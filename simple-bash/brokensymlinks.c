#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
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
    struct dirent * dir;
    struct stat sbuf;
    d = opendir(name);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            lstat(dir->d_name, &sbuf);
            if (!access(dir->d_name, F_OK) && S_ISLNK(sbuf.st_mode))
            {
                printf("%s\n", s->d_name);
            }
                }
            closedir(d);
        }
        struct stat stat;
        if(
            for (

                }

/* int main(void) */
/* { */
/*   DIR           *d; */
/*   struct dirent *dir; */
/*   d = opendir("."); */
/*   if (d) */
/*   { */
/*     while ((dir = readdir(d)) != NULL) */
/*     { */
/*       printf("%s\n", dir->d_name); */
/*     } */

/*     closedir(d); */
/*   } */

/*   return(0); */
/* } */

            int main(int argc, char ** argv)
            {
                char * path = argv[1];
                brokensymlinks(path);
                return 0;
            }
