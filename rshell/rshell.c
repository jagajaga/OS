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
            //close(0);
            //close(1);
            //close(2);

            write(acceptfd, "foo\n", 4);
//            if (close(acceptfd) == -1)
//           {
//               fprintf(stderr, "Could not close\n");
//               exit(EXIT_FAILURE);
//           }

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
                //close(master);
                //close(acceptfd);
				int flags;
				flags = fcntl(master, F_GETFL);
				if (flags < 0)
					exit (1);
				flags = flags | O_NONBLOCK;
				fcntl(master, F_SETFL, flags);
				flags = fcntl(acceptfd, F_GETFL);
				if (flags < 0)
					exit (1);
				flags = flags | O_NONBLOCK;
				fcntl(acceptfd, F_SETFL, flags);
                char * buffer = malloc(4096);
				int read_result_fd;

                while (1)
                {
                    read_result_fd = read(master, buffer, 4096);

                    if (read_result_fd == 0)
                        exit(1);
					else if (read_result_fd < 0)
						if (errno != EWOULDBLOCK && errno != EAGAIN)
							exit(1);


                    write(acceptfd, buffer, read_result_fd);
                    read_result_fd = read(acceptfd, buffer, 4096);

                    if (read_result_fd == 0)
                        exit(1);
					else if (read_result_fd < 0)
						if (errno != EWOULDBLOCK && errno != EAGAIN)
							exit(1);

                    write(master, buffer, read_result_fd);
                }
            }
            else
            {
                //dup2(acceptfd, 0);
                //dup2(acceptfd, 1);
                //dup2(acceptfd, 2);
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

            /*
                //close(master);
                close(slave);
            	while(1)
            	{
            //		int read_result_master = read(master, buffer, read_result_fd);
            //		if (read_result_master == -1)
            //		{
            //			fprintf(stderr, "Read error\n");
            //			close(master);
            //		}
            //		write(acceptfd, buffer, read_result_master);

            	}
            }
            else
            {
            }
            */

        }
    }

    return 0;
}
