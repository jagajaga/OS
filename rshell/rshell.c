#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pty.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#define BUF_SIZE 500

void my_exit (char ** buffer, int fd, int code)
{

    close(fd);
    free(buffer[0]);
    free(buffer[1]);
    free(buffer);
    exit(code);
}

int main (int argc, char ** argv)
{
    if (!fork())
    {
        setsid();
        struct addrinfo hints;
        struct addrinfo * result, *rp;
        int sfd, s;
        struct sockaddr peer_addr;
        socklen_t peer_addr_len;

        int option = 1;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        hints.ai_protocol = 0;          /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        s = getaddrinfo(NULL, "8822", &hints, &result);

        if (s != 0)
        {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            exit(EXIT_FAILURE);
        }

        rp = result;
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);

        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *) & option, sizeof(option)) != 0)
        {
            fprintf(stderr, "Could not set options to socket\n");
            exit(EXIT_FAILURE);
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != 0)
        {
            fprintf(stderr, "Could not bind\n");
            exit(EXIT_FAILURE);
        }

        freeaddrinfo(result);           /* No longer needed */


        if (listen(sfd, 100) != 0)
        {
            fprintf(stderr, "Could not start listen\n");
            exit(EXIT_FAILURE);
        }

        int acceptfd = accept(sfd, &peer_addr, &peer_addr_len);

        if (acceptfd == -1)
        {
            fprintf(stderr, "Could not accept\n");
            exit(EXIT_FAILURE);
        }

        if (fork())
        {
            if (close(acceptfd) == -1)
            {
                fprintf(stderr, "Could not close\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            int master, slave;
            char buff[4096];

            if ( openpty(&master, &slave, buff, NULL, NULL) == -1)
            {
                fprintf(stderr, "Could not open pty\n");
                exit(EXIT_FAILURE);
            }

            if (fork())
            {
                close(slave);
                int flags;
                flags = fcntl(master, F_GETFL);

                if (flags < 0)
                {
                    exit (1);
                }

                flags = flags | O_NONBLOCK;
                fcntl(master, F_SETFL, flags);
                flags = fcntl(acceptfd, F_GETFL);

                if (flags < 0)
                {
                    exit (1);
                }

                flags = flags | O_NONBLOCK;
                fcntl(acceptfd, F_SETFL, flags);
                char ** buffer = malloc(2 *sizeof(char * ));
                buffer[0] = malloc(4096);
                buffer[1] = malloc(4096);
                int buffer_size[2];
                buffer_size[0] = 0;
                buffer_size[1] = 0;
                int dead = 0;
                struct pollfd fds[2];
                const int timeout_msecs = -1;
                int ret;

                const int events = POLLIN | POLLERR | POLLHUP | POLLNVAL;
                fds[0].fd = master;
                fds[0].events = events;
                fds[1].fd = acceptfd;
                fds[1].events = events;
                sleep(1);

                write(acceptfd, "Hello! You are in rshell\n", 25);

                while (1)
                {
                    ret = poll(fds, 2, timeout_msecs);

                    if (ret > 0)
                    {
                        int i;

                        for (i = 0; i < 2; i++)
                        {
                            int j = i == 0 ? 1 : 0;

                            if (fds[i].revents & POLLIN)
                            {
                                int read_result = read(fds[i].fd, buffer[i], 4096 - buffer_size[i]);

                                if (read_result == 0)
                                {
                                    close(fds[i].fd);
                                    dead = 1;
                                }
                                else if (read_result < 0)
                                    if (errno != EWOULDBLOCK && errno != EAGAIN)
                                    {
                                        my_exit(buffer, fds[i].fd, 1);
                                    }

                                if (read_result > 1)
                                {
                                    buffer_size[i] += read_result;
                                }
                            }

                            if (fds[j].revents & POLLOUT)
                            {
                                int write_result = write(fds[j].fd, buffer[i], buffer_size[i]);
                                buffer_size[i] -= write_result;

                                if (write_result < 0)
                                    if (errno != EWOULDBLOCK && errno != EAGAIN)
                                    {
                                        my_exit(buffer, fds[j].fd, 1);
                                    }
                            }

                            if (buffer_size[i] > 0)
                            {
                                fds[j].events = fds[j].events | POLLOUT;
                            }
                            else
                            {
                                if (fds[j].events & POLLOUT)
                                {
                                    fds[j].events = fds[j].events ^ POLLOUT;

                                    if (dead)
                                    {
                                        my_exit(buffer, fds[j].fd, 1);
                                    }
                                }
                            }

                        }

                        for (i = 0; i < 2; i++)
                        {
                            int const else_polls = POLLERR | POLLHUP | POLLNVAL;

                            if (fds[i].revents & else_polls)
                            {
                                my_exit(buffer, fds[i].fd, 1);
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                free(buffer[0]);
                free(buffer[1]);
                free(buffer);
                exit(0);
            }
            else
            {
                dup2(slave, 0);
                dup2(slave, 1);
                dup2(slave, 2);
                close(master);
                close(slave);
                close(acceptfd);
                setsid();
                int fd = open(buff,  O_RDWR);
                close(fd);
                execl("/bin/zsh", "zsh", NULL);
                exit(1);
            }
        }
    }

    return 0;
}
