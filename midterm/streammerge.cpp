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
    stream(int fd, char delimiter) : fd(fd), count(0), buffer((char *) malloc(buffer_size)), delimiter(delimiter) {}
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
    int buffer_result;

    while(1)
    {
        buffer_result = read(fd, buffer + count, buffer_size - count);
        int from = count;
        count += buffer_result;
        int pos = 0;

        if (buffer_result == 0)
        {
            return get_string_at_pos(pos, from, count);
        }

        if (buffer_result < 0)
        {
            break;
        }

        return get_string_at_pos(pos, from, count - from);
    }
    return boost::none;
}

class streammerge
{
private:
    bool current_fd;
    char delimiter;
    stream s1, s2;
public:
    streammerge(int fd1, int fd2, char delimiter) : current_fd(1), delimiter(delimiter), s1(fd1, delimiter), s2(fd2, delimiter) {}

    boost::optional<std::string> get_string();

    ~streammerge()
    {
    };
};

boost::optional<std::string> streammerge::get_string()
{
    current_fd = !current_fd;

    if (current_fd)
    {
        return s2.get_string();
    }
    else
    {
        return s1.get_string();
    }

    return boost::none;
}



int main(int argc, char ** argv)
{
    if (argc < 4)
    {
        std::cout << "Whrong argument size";
        return 1;
    }

    int file1 = open(argv[1], O_RDONLY);
    int file2 = open(argv[2], O_RDONLY);

    if (file1 == -1 || file2 == -1)
    {
        std::cout << "Couldn't open file \"" << (file1 == 1 ? argv[1] : argv[2]) << "\"";
		return 2;
    }

    streammerge sm(file1, file2, argv[3][0]);
    boost::optional<std::string> result;

    for (int i = 0; i < 10; i++)
        if (result = sm.get_string())
        {
            std::cout << *result << "\n";
        }
}
