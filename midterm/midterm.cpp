#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <deque>
#include <string>
#include <iostream>
#include <stdio.h>

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


int find_separator(char separator, char * buffer, int size)
{
    if (size > 0)
    {
        for( int i = 0; i < size; i++)
        {
            if (buffer[i] == separator)
            {
                return i;
            }
        }
    }
    return -1;
}

void one_run(std::deque <std::string> & v)
{
    char ** command = (char **) malloc((v.size() + 1) * (sizeof(char *)));
    for (int i = 0; (unsigned) i < v.size(); i++)
    {
        command[i] = &v[i][0];
    }
    command[v.size()] = NULL;
    int pid = fork();
    if (pid)
    {
        pid_t tpid;
        int status;
        do
        {
            tpid = wait(&status);
        }
        while (tpid != pid);
        free(command);
        _exit(0);
    }
    else
    {
        if (execvp(&v[0][0], command) == -1)
        {
            _exit(1);
        }
    }
}


void run(std::deque<std::deque<std::string> > & v)
{

    if (v.size() < 1)
    {
        _exit(1);
    }
    if (v.size() == 1)
    {
        one_run(v[0]);
    }
    else
    {
        int pipefd[2];
        pipe(pipefd);

        if (fork())
        {
            dup2(pipefd[0], 0);
            close(pipefd[0]);
            close(pipefd[1]);
            v.pop_front();
            run(v);
        }
        else
        {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);
            one_run(v[0]);
        }
    }
}

int min (int a, int b)
{
    if (a < b)
        return a;
    return b;
}

void split(const std::string & txt, std::deque<std::string> & strs, char ch)
{
    unsigned int pos = txt.find(ch);
    unsigned int initial_pos = 0;
    strs.clear();
    while (pos < txt.size())
    {
        strs.push_back(txt.substr(initial_pos, pos - initial_pos) );
        initial_pos = pos + 1;
        pos = txt.find(ch, initial_pos);
    }
    for (int i = 0; (unsigned) i < strs.size(); i++)
    {
        if (strs[i][0] == '$')
        {
            char * c = (char *) malloc (strs[i].size() - 1);
            memcpy(c, &strs[i][1], strs[i].size() - 1);
            char * tmp_var = getenv(c);
            if (tmp_var == NULL )
            {
                strs[i] = "";
                return;
            }
            strs[i] = tmp_var;
        }
    }
}

int main(int argc, char ** argv)
{
    char * buffer;
    char separator = '|';
    int buffer_size = 4096;
    int buffer_result;

    buffer = (char *) malloc(buffer_size);
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
            buffer_result = read(0, buffer + count, buffer_size - count);
            if (buffer_result == 0)
            {
                eof = 1;
            }
            if (buffer_result < 0)
            {
                break;
            }
            int from = count;
            count += buffer_result;
            int pos;
            std::deque < std::deque < std::string > > global_v;
            while ((pos = find_separator(separator, buffer, count - from)) >= 0)
            {
                char * temp_name = (char *) malloc(pos);
                if (buffer[0] == ' ')
                {
                    memcpy(temp_name, buffer + 1, pos - 1);
                }
                else
                {
                    memcpy(temp_name, buffer, pos);
                }
                std::deque<std::string> v;
                std::string s(temp_name);
                split(s, v, ' ');
                global_v.push_back(v);
                pos++;
                if (count - pos < 0)
                {
                    break;
                }
                memmove(buffer, buffer + pos, count - pos);
                from = 0;
                count -= pos;
            }
            run(global_v);
        }
        if (count > 0)
        {
            if (count + 1 >= buffer_size)
            {
                return -1;
            }
        }
        break;
    }
    free(buffer);
    return 0;
}
