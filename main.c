#include <unistd.h>

void _write(int to, char * what, size_t size)
{
    size_t writen = 0;

    while (writen <= size)
    {
        writen += write(to, what + writen, size);

        if (writen == 0)
        {
			
        }
        size -= writen;
    }
}
int main (int argc, char * argv[])
{
    int pipefd[2];
    pipe (pipefd);
    _write(1, "hello\n", 6);

    if (fork())
    {
        dup2(pipefd[1], 1);
        close(pipefd[0]);
        close(pipefd[1]);
        execl("/bin/ls", "ls", NULL);
    }
    else
    {
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        close(pipefd[1]);
        execl("/usr/bin/grep", "grep", "file", NULL);
    }

    return 0;
}
