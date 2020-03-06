#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  /* The user will type in a user name on one line followed by a password
     on the next.
     DO NOT add any prompts.  The only output of this program will be one
	 of the messages defined above.
   */

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }

  // TODO
  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    exit(1);
  }

  int r = fork();

  if (r < 0) {
    perror("fork");
    exit(1);
  } else if (r == 0) {

    // Child will write to stdin through pipe
    dup2(pipe_fd[0], fileno(stdin));

    if (write(pipe_fd[1], user_id, 10) == -1) {
      perror("child write to stdin");
    }

    if (write(pipe_fd[1], password, 10) == -1) {
      perror("child second write to stdin");
    }

    if (close(pipe_fd[1]) == -1) {
      perror("close write");
    }

    if (execl("./validate", "validate", NULL) == -1) {
      perror("execl");
      exit(1);
    }

  } else {
    int status;
    int validate_exit;

    if ((wait(&status)) == -1) {
      perror("wait");
    } else {
      if (WIFEXITED(status)) {
        validate_exit = WEXITSTATUS(status);
        if (validate_exit == 0) {
          printf(SUCCESS);
        } else if (validate_exit == 1) {
          printf("error\n");
        } else if (validate_exit == 2) {
          printf(INVALID);
        } else if (validate_exit == 3) {
          printf(NO_USER);
        }
      } else if (WIFSIGNALED(status)) {
        exit(1);
      }
    }
  }

  return 0;
}
