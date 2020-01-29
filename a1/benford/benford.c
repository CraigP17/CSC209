#include <stdio.h>
#include <stdlib.h>

#include "benford_helpers.h"

/*
 * The only print statement that you may use in your main function is the following:
 * - printf("%ds: %d\n")
 *
 */
int main(int argc, char **argv) {

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "benford position [datafile]\n");
        return 1;
    }

    // TODO: Implement.
    int index = strtol(argv[1], NULL, 10);
    int *output = malloc(sizeof(int) * 10);
    for (int i = 0; i < 10; i++) {
      output[i] = 0;
    }
    if (argc == 3) {
      FILE *filetxt;
      filetxt = fopen(argv[2], "r");
      int current;
      while (fscanf(filetxt, "%d", &current) == 1) {
        add_to_tally(current, index, output);
      }
      fclose(filetxt);
    } else if (argc == 2) {
      int current;
      int scan = fscanf(stdin, "%d", &current);
      while (scan != EOF) {
        add_to_tally(current, index, output);
        scan = fscanf(stdin, "%d", &current);
      }
    }
    for (int i = 0; i < 10; i++) {
      printf("%ds: %d\n", i, output[i]);
    }
    free(output);
    return 0;
}
