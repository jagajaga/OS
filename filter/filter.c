#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

char zero = '\0';
char new_line = '\n';

size_t _write(int fd, char * buffer, size_t size)
{
    size_t current = 0;
    while (current < size)
    {
        size_t result = write(fd, buffer + current, size);
        if (result < 1)
        {
            return current;
        }
        current += result;
        size -= result;
    }
    return current;
}

size_t _read(int fd, char * buffer, size_t size)
{
    size_t current = 0;
    while (current < size)
    {
        size_t result = read(fd, buffer + current, size);
        if (result < 1)
        {
            return current;
        }
        current += result;
        size -= current;
    }
    return current;
}

int find_separator(char separator, char * buffer, int from, int size)
{
    int pos = from;
    while (pos < size && buffer[pos] != separator)
    {
        pos++;
    }
    if (pos == size)
    {
        return -1;
    }
    return pos;
}

void run_command(char ** command, char * buffer, int from, int size, char separator)
{
    int pid = fork();
    int child_status;
    if (pid)
    {
        pid_t tpid;
        do
        {
            tpid = wait(&child_status);
            if (WIFEXITED(child_status) && !WEXITSTATUS(child_status))
            {
                _write(1, buffer + from, size);
		_write(1, &separator, 1);
            }
        } while (tpid != pid);
    }
    else
    {
        execvp(command[0], command);
        exit(0);
    }
}

int main(int argc, char ** argv)
{
    int opt;
    char * buffer;
    char c, separator = new_line;
    int buffer_size = 4096;
    int buffer_result;
    int count = 0;
    char ** command;

    while ((opt = getopt(argc, argv, "nzb:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            separator = new_line;
            break;
        case 'z':
            separator = zero;
            break;
        case 'b':
            buffer_size = atoi(optarg);
            break;
        }
    }
    if (optind < argc) {
        command = malloc((argc - optind + 2) * sizeof(char *));
        int i = 0;
        for (i = optind; i < argc; i++)
        {
            command[i - optind] = argv[i];
        }
        command[argc - optind + 2] = 0;
    }
    buffer = malloc(buffer_size);
    while (1)
    {
        int eof = 0;
        int count = 0;
        while(!eof)
        {
            if (count >= buffer_size)
            {
                return -1;
            }
            buffer_result = _read(0, buffer + count, buffer_size - count);
            if (buffer_result == 0)
            {
                eof = 1;
            }
            int from = count;
            count += buffer_result;
            int pos;
            while ((pos = find_separator(separator, buffer, from, count - from)) >= 0)
            {
                char ** temp_command = command;
		int i;
		char * temp_name = malloc(pos);
		memcpy(temp_name, buffer, pos);
                temp_command[argc - optind] = temp_name;
                /* for (i = 0; i < argc - optind + 2; i++) */
                /*     printf("%s ", temp_command[i]); */
		/* printf("\n"); */
                run_command(temp_command, buffer, 0, pos, separator);
                memmove(buffer, buffer + (++pos), count - pos);
                from = 0;
                count -= pos;
            }
        }
        if (count > 0)
        {
            if (count + 1 >= buffer_size)
            {
                return -1;
            }
            buffer[count + 1] = separator;
            char ** temp_command = command;
            temp_command[argc - optind + 1] = buffer;
            run_command(temp_command, buffer, 0, count + 1, separator);
        }
        break;
    }
    free(buffer);
    return 0;
}
