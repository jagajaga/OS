#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/optional.hpp>

size_t _write(int fd, const char * buffer, size_t size)
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

int find_delimiter(char delimiter, char * buffer, int size)
{
    if (size > 0)
    {
        for( int i = 0; i < size; i++)
        {
            if (buffer[i] == delimiter)
            {
                return i;
            }
        }
    }
    return -1;
}

class stream
{
private:
    int fd;
    int count;
    char * buffer;
    char delimiter;
    static const int buffer_size = 4096;
    boost::optional<std::string> get_string_at_pos(int, int, int);
public:
    stream(int fd, char delimiter) : fd(fd), count(0), buffer((char*) malloc(buffer_size)), delimiter(delimiter){}
    boost::optional<std::string> get_string();
    ~stream()
        {
            free(buffer);
        }
};

boost::optional<std::string> stream::get_string_at_pos(int pos, int from, int size)
{
    if ((pos = find_delimiter(delimiter, buffer, size)) >= 0)
    {
        char * temp_name = (char *) malloc(pos);
        memcpy(temp_name, buffer, pos);
        pos++;
        if (count - pos < 0)
        {
            return boost::none;
        }
        memmove(buffer, buffer + pos, count - pos);
        from = 0;
        count -= pos;
        return std::string(temp_name);
    }
    return boost::none;
}

boost::optional<std::string> stream::get_string()
{
    char c;
    int buffer_result;
    int eof = 0;
    while(!eof)
    {
        buffer_result = read(fd, buffer + count, buffer_size - count);
        if (count >= buffer_size)
        {
            return boost::none;
        }
        int from = count;
        count += buffer_result;
        int pos;
        if (buffer_result == 0)
        {
            return get_string_at_pos(pos, from, count);
            eof = 1;
        }
        if (buffer_result < 0)
        {
            break;
        }
        return get_string_at_pos(pos, from, count - from);
    }
    return boost::none;
}

void run(char ** command, int argc, std::string s)
{
    std::string str = s;
    command[argc - 1] = &str[0];
    // for (int i = 0; i < argc + 1; i++)
    // {
    //     std::cerr << command[i] << ' ';
    // }
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
                _write(1, command[argc - 1], str.size());
                const char * new_line = "\n";
                _write(1, new_line, 1);
            }
        } while (tpid != pid);
    }
    else
    {
        execvp(command[0], command);
        exit(0);
    }
}


class run_command
{
private:
    stream s;
    char ** command;
    int argc;
public:
    run_command(int argc, char ** argv) : s(0, '\n'), argc(argc)
        {
            command = (char **) (malloc((argc + 1) * sizeof(char *)));
            for (int i = 1; i < argc; i++)
            {
                command[i -1] = argv[i];
            }
            command[argc] = 0;
        }
    void run_from_stream()
        {
	    for (int i = 0; i < 20; i++)
	    {
            if (boost::optional<std::string> ss = s.get_string())
            {
                run(command, argc, *ss);
            }
	    }
        }
};


int main(int argc, char ** argv)
{
    run_command rc(argc, argv);
    rc.run_from_stream();
    rc.run_from_stream();
    rc.run_from_stream();
    rc.run_from_stream();
    rc.run_from_stream();
    rc.run_from_stream();
}
