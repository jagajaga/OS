#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/optional.hpp>

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



class streammerge
{
private:
    int fd1, fd2;
    bool current_fd;
    char delimiter;
    char * buffer1;
    char * buffer2;
    int count1;
    int count2;
    static const int buffer_size = 4096;
    boost::optional<std::string> get_string_at_pos(char *, int, int, int, int);
public:
    streammerge(int fd1, int fd2, char delimiter) : fd1(fd1), fd2(fd2), current_fd(0), delimiter(delimiter), buffer1((char *) malloc(buffer_size)), buffer2((char *) malloc(buffer_size)), count1(0), count2(0) {}

    boost::optional<std::string> get_string();

    ~streammerge()
        {
            free(buffer1);
            free(buffer2);
        };
};

boost::optional<std::string> streammerge::get_string_at_pos(char * buffer, int pos, int from, int count, int size)
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
        if(!current_fd)
        {
            count2 = count;
        }
        else
        {
            count1 = count;
        }
        return std::string(temp_name);
    }
    return boost::none;
}

boost::optional<std::string> streammerge::get_string()
{
    char c;
    int buffer_result;
    int eof = 0;
    int count = 0;
    while(!eof)
    {
        char * buffer;
        if (current_fd)
        {
            count = count2;
            buffer = buffer2;
            buffer_result = read(fd2, buffer + count, buffer_size - count);
        }
        else
        {
            count = count1;
            buffer = buffer1;
            buffer_result = read(fd1, buffer + count, buffer_size - count);
        }
        if (count >= buffer_size)
        {
            return boost::none;
        }
        current_fd = !current_fd;
        int from = count;
        count += buffer_result;
        int pos;
        if (buffer_result == 0)
        {
            return get_string_at_pos(buffer, pos, from, count, count);
            eof = 1;
        }
        if (buffer_result < 0)
        {
            break;
        }
        return get_string_at_pos(buffer, pos, from, count, count - from);
        // if ((pos = find_delimiter(delimiter, buffer, count - from)) >= 0)
        // {
        //     char * temp_name = (char *) malloc(pos);
        //     memcpy(temp_name, buffer, pos);
        //     pos++;
        //     if (count - pos < 0)
        //     {
        //         break;
        //     }
        //     memmove(buffer, buffer + pos, count - pos);
        //     from = 0;
        //     count -= pos;
        //     if(!current_fd)
        //     {
        //         count2 = count;
        //     }
        //     else
        //     {
        //         count1 = count;
        //     }
        //     return std::string(temp_name);
        // }
    }
    return boost::none;
}



int main(int argc, char ** argv)
{
    streammerge sm(open(argv[1], O_RDONLY), open(argv[2], O_RDONLY), argv[3][0]);
    boost::optional<std::string> result;
    for (int i = 0; i < 10; i++)
        if (result = sm.get_string())
        {
            std::cout << *result << "\n";
        }
}
