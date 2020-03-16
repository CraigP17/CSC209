#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"


/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {

    if ((n < 4) || (pdmax == 0)) {
      return closest_serial(p, n);
    }

    // Find the middle point of the Points to split into two halves
    int mid = n / 2;
    struct Point mid_point = p[mid];

    // Create a pipe for child 1 to communicate with the parent
    int pipe_1[2];
    if (pipe(pipe_1) == -1) {
      perror("pipe 1");
      exit(1);
    }

    // Forking first child for left distances
    int child1 = fork();
    if (child1 < 0) {
      perror("fork");
      exit(1);
    } else if (child1 == 0) {
      // Child Process for Finding Distance
      double left_min;
      left_min = closest_parallel(p, mid, pdmax - 1, pcount);

      // Child writes to Parent about Left Distance through Pipe
      // Child writes so close reading end
      if (close(pipe_1[0]) == -1) {
        perror("Close Child Read End");
        exit(1);
      }

      // Child now writes the value in pipe
      if (write(pipe_1[1], &left_min, sizeof(double)) != sizeof(double)) {
        perror("Write from Child");
        exit(1);
      }

      // Close Write End
      if (close(pipe_1[1]) == -1) {
        perror("Close Child Write End");
        exit(1);
      }

      // Exit with number of worker processes
      exit(*pcount + 1);

    } else {
      // Parent Process

      // Create a Pipe for Second Child
      int pipe_2[2];
      if (pipe(pipe_2) == -1) {
        perror("pipe 2");
        exit(1);
      }

      // Forking second child for right distances
      int child2 = fork();
      if (child2 < 0) {
        perror("fork 2");
        exit(1);
      } else if (child2 == 0) {
        // Child 2 Process for Finding Distance
        double right_min;
        right_min = closest_parallel(&p[mid], n - mid, pdmax - 1, pcount);

        // Child writes to Parent about Right Distance through Pipe
        // Child writes so close reading end
        if (close(pipe_2[0]) == -1) {
          perror("Close Child 2 Read End");
          exit(1);
        }

        // Child now writes the value in pipe
        if (write(pipe_2[1], &right_min, sizeof(double)) != sizeof(double)) {
          perror("Write from Child 2");
          exit(1);
        }

        // Close Write End
        if (close(pipe_2[1]) == -1) {
          perror("Close Child 2 Write End");
          exit(1);
        }

        // Exit with number of worker processes + 1 for current
        exit(*pcount + 1);

      } else {
        // Parent of Both Children

        // Wait for Child Processes to Finish
        int child_stat1, child_stat2;
        if (wait(&child_stat1) == -1) {
          perror("wait 1");
          exit(1);
        }
        // Get exit signal from Child 1 for PCount
        if (WIFEXITED(child_stat1)) {
          *pcount += WEXITSTATUS(child_stat1);
        }

        if (wait(&child_stat2) == -1) {
          perror("wait 2");
          exit(1);
        }
        // Get exit signal from Child 2 for PCount
        if (WIFEXITED(child_stat2)) {
          *pcount += WEXITSTATUS(child_stat2);
        }

        // Read Min Distance that Children Wrote to Pipe
        double min_left, min_right;

        // Close Write from Child 1 Pipe
        if (close(pipe_1[1]) == -1) {
          perror("Close read from Child 1");
          exit(1);
        }
        // Read from Child 1 Pipe
        if (read(pipe_1[0], &min_left, sizeof(double)) != sizeof(double)) {
          perror("Read from Child 1 Pipe");
          exit(1);
        }
        // Close read from Child 1 Pipe
        if (close(pipe_1[0]) == -1) {
          perror("Close read from Child 1");
          exit(1);
        }

        // Close Write from Child 2 Pipe
        if (close(pipe_2[1]) == -1) {
          perror("Close read from Child 2");
          exit(1);
        }
        // Read from Child 2 Pipe
        if (read(pipe_2[0], &min_right, sizeof(double)) != sizeof(double)) {
          perror("Read from Child 2 Pipe");
          exit(1);
        }
        // Close read from Child 2 Pipe
        if (close(pipe_2[0]) == -1) {
          perror("Close read from Child 2");
          exit(1);
        }

        // Find minimum of distance of two points,
        // one from left points and one from right points
        double d = min(min_left, min_right);

        struct Point *middle = malloc(sizeof(struct Point) * n);
        if (middle == NULL) {
          perror("malloc");
          exit(1);
        }

        int j = 0;
        for (int i = 0; i < n; i++) {
            if (abs(p[i].x - mid_point.x) < d) {
                middle[j] = p[i], j++;
            }
        }

        // Find the closest points in strip.  Return the minimum of d and closest distance in strip[].
        double new_min = min(d, strip_closest(middle, j, d));
        free(middle);

        return new_min;
      }
    }

    return 0.0;
}
