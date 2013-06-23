#include "epollfd.h"

#include <iostream>

int main()
{
    epollfd ed;
    std::function<void()> test = [&ed, &test]()
    {
        std::cerr << "OK\n";
        ed.subscribe(0, EPOLLIN, test, []() {});
    };
    ed.subscribe(0, EPOLLIN, test, []() {});

    while (1)
    {
        ed.cycle();
    }
}
