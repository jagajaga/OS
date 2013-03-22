#include <unistd.h>

int main (int argc, char * argv[]) {
	int pipefd[2];
	pipe (pipefd);

	if (fork()) {
		dup2(pipefd[1], 1);
		close(pipefd[0]);
		close(pipefd[1]);
		execl("/bin/ls", "ls", NULL);
	} else {
		dup2(pipefd[0], 0);
		close(pipefd[0]);
		close(pipefd[1]);
		execl("/usr/bin/grep", "grep", "file", NULL);
	}
	return 0;
}
