/* The purpose of this program is to practice writing signal handling
 * functions and observing the behaviour of signals.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Message to print in the signal handling function. */
#define MESSAGE "%ld reads were done in %ld seconds.\n"

/* Global variables to store number of read operations and seconds elapsed.
 */
long num_reads, seconds;

unsigned int alarm(unsigned int seconds) {
    struct itimerval old, new;
    new.it_interval.tv_usec = 0;
    new.it_interval.tv_sec = 0;
    new.it_value.tv_usec = 0;
    new.it_value.tv_sec = (long int) seconds;
    if (setitimer (ITIMER_PROF, &new, &old) < 0) {
      return 0;
    } else {
      return old.it_value.tv_sec;
    }
}

void handler(int sig) {
    printf(MESSAGE, num_reads, seconds);
    exit(0);
}

/* The first command-line argument is the number of seconds to set a timer to run.
 * The second argument is the name of a binary file containing 100 ints.
 * Assume both of these arguments are correct.
 */

int main(int argc, char **argv) {

    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGPROF,&act,NULL);

    if (argc != 3) {
        fprintf(stderr, "Usage: time_reads s filename\n");
        exit(1);
    }
    seconds = strtol(argv[1], NULL, 10);

    FILE *fp;
    if ((fp = fopen(argv[2], "r")) == NULL) {
      perror("fopen");
      exit(1);
    }

    alarm(seconds);
    /* In an infinite loop, read an int from a random location in the file,
     * and print it to stderr.
     */
    for (;;) {
      int randot = random() % 100;
      int reader;
      fseek(fp, randot * sizeof(int), SEEK_SET);
      fread(&reader, sizeof(int), 1, fp);
      fprintf(stderr, "%d\n", reader);

      num_reads++;
    }
    return 1; // something is wrong if we ever get here!
}
