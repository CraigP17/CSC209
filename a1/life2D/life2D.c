#include <stdio.h>
#include <stdlib.h>

#include "life2D_helpers.h"


int main(int argc, char **argv) {

    if (argc != 4) {
        fprintf(stderr, "Usage: life2D rows cols states\n");
        return 1;
    }

    // TODO: Implement.
    int row = strtol(argv[1], NULL, 10);
    int col = strtol(argv[2], NULL, 10);
    int state = strtol(argv[3], NULL, 10);
    int max = (row * col);
    int oboard[max];

    int scan = scanf("%d", &oboard[0]);
    int i = 1;
    while (scan != EOF) {
        scan = scanf("%d", &oboard[i]);
        i++;
    }

    print_state(oboard, row, col);
    for (int j = 1; j < state; j++) {
        update_state(oboard, row, col);
        print_state(oboard, row, col);
    }
}
