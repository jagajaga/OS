#include "epollfd.h"

#define MAX_SIZE 10

epollfd::epollfd()
{
    epoll_fd = epoll_create(MAX_SIZE);

    if (epoll_fd == -1)
    {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }
}

epollfd::~epollfd()
{
    close(epoll_fd);
};

void epollfd::subscribe(int fd, int what, std::function<void()> const& cont_ok, std::function<void()> const& cont_err)
{
    my_map.count(fd);

    if (my_map.find(fd) == my_map.end())
    {
        epoll_event event;

        event.data.fd = fd;
        event.events = what;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event))
        {
            perror("epoll_add");
            exit(EXIT_FAILURE);
        }

        cont tmp = {cont_ok, cont_err};
        my_map[fd].insert({what, tmp});
    }
    else
    {
        if ((*my_map.find(fd)).second.find(what) == (*my_map.find(fd)).second.end())
        {
            epoll_event event;
            event.events = what;
            event.data.fd = fd;

            if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event))
            {
                perror("epoll_mod");
                exit(EXIT_FAILURE);
            }

            my_map[fd][what].ok = cont_ok;
            my_map[fd][what].err = cont_err;
        }
        else
        {
            throw std::runtime_error("dublicate");
        }

    }
}

void epollfd::unsubscribe(int fd, int what)
{

    if (my_map.find(fd) == my_map.end())
    {
        throw std::runtime_error("No such fd found");
    }
    else
    {

        if ((*my_map.find(fd)).second.find(what) == (*my_map.find(fd)).second.end())
        {
            throw std::runtime_error("No such event found");
        }

        (*my_map.find(fd)).second.erase(what);

        if ((*my_map.find(fd)).second.empty())
        {

            if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL))
            {
                perror("epoll_del");
                exit(EXIT_FAILURE);
            }

            my_map.erase(my_map.find(fd));

        }
        else
        {
            epoll_event event;
            event.events = what;
            event.data.fd = fd;

            if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event))
            {
                perror("epoll_mod");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void epollfd::cycle()
{
    epoll_event events[MAX_SIZE];
    int n = epoll_wait (epoll_fd, events, MAX_SIZE, -1);

    if (n == -1)
    {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++)
    {
        auto& event = events[i];
        int fd = event.data.fd;
        auto fd_map = my_map.find(fd);

        for (auto it = fd_map->second.begin(); it != fd_map->second.end(); it = std::next(it))
        {
            auto cont_map = (*it).second;
            auto what = (*it).first;
            unsubscribe(fd, what);

            if ((event.events & (EPOLLERR | EPOLLHUP))
                    && !(event.events & (EPOLLIN | EPOLLHUP)))
            {
                cont_map.err();
            }
            else
            {
                if (what & event.events)
                {
                    cont_map.ok();
                }
            }

            if (fd_map->second.empty())
            {
                break;
            }
        }
    }
}

