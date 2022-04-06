/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2018-2022 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

int main(int argc, char *argv[], char *envp[])
{
	int fd, input = -1, pipes[2] = { 0 }, status;
	pid_t compiler_pid;
	char *output_path;

	if (argc > 1 && argv[argc - 1][0] != '-') {
		input = open(argv[1], O_RDONLY);
		++argv;
		--argc;
		if (input < 0) {
			perror("Error: unable to open input file");
			return 1;
		}
		if (pipe(pipes) < 0) {
			perror("Error: unable to open filter pipe");
			return 1;
		}
	}

	fd = memfd_create("cscript", 0);
	if (fd < 0) {
		perror("Error: unable to create memfd");
		return 1;
	}
	if (asprintf(&output_path, "/proc/self/fd/%d", fd) < 0) {
		perror("Error: unable to allocate memory for fd string");
		return 1;
	}

	compiler_pid = fork();
	if (compiler_pid < 0) {
		perror("Error: unable to fork for compiler");
		return 1;
	}

	if (compiler_pid == 0) {
		const char *cc = getenv("CC") ?: "gcc";
		close(input);
		if (pipes[0] != 0) {
			close(pipes[1]);
			if (dup2(pipes[0], 0) < 0) {
				perror("Error: unable to duplicate pipe fd");
				_exit(1);
			}
			close(pipes[0]);
		}
		execlp(cc, cc, "-pipe", "-xc", "-o", output_path, "-O2", "-march=native", "-", NULL);
		_exit(1);
	}

	if (input != -1) {
		char beginning[2];
		ssize_t len;

		close(pipes[0]);
		len = read(input, beginning, 2);
		if (len < 0) {
			perror("Error: unable to read from input file");
			return 1;
		} else if (len == 2 && beginning[0] == '#' && beginning[1] == '!')
			len = write(pipes[1], "//", 2);
		else if (len > 0)
			len = write(pipes[1], beginning, len);
		if (len < 0) {
			perror("Error: unable to write input preamble");
			return 1;
		}
		if (splice(input, NULL, pipes[1], NULL, 0x7fffffff, 0) < 0) {
			perror("Error: unable to splice input to compiler child");
			return 1;
		}
		close(pipes[1]);
	}

	if (waitpid(compiler_pid, &status, 0) != compiler_pid || (!WIFEXITED(status) || WEXITSTATUS(status))) {
		fprintf(stderr, "Error: compiler process did not complete successfully\n");
		return 1;
	}

	if (fexecve(fd, argv, envp) < 0) {
		perror("Error: could not execute compiled program");
		return 1;
	}
	return 0;
}
