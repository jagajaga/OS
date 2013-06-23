#pragma once
#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <unordered_map>
#include <stdexcept>


class epollfd
{
private:
    int epoll_fd;
    struct cont
    {
        std::function<void()> ok;
        std::function<void()> err;
    };
    std::unordered_map<int, std::unordered_map<int, cont> > my_map;

public:
    epollfd();

    ~epollfd();

    epollfd(epollfd const&) = delete;

    epollfd& operator= (epollfd const&) = delete;

    void subscribe(int, int, std::function<void()> const&, std::function<void()> const&);

    void unsubscribe(int, int);

    void cycle();
};
