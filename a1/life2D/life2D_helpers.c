#include <stdio.h>
#include <stdlib.h>


void print_state(int *board, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows * num_cols; i++) {
        printf("%d", board[i]);
        if (((i + 1) % num_cols) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

int check_around(int *cboard, int index, int col) {
    int count;
    count = cboard[index - col - 1] + cboard[index - col] + cboard[index - col + 1] +
              cboard[index - 1] + cboard[index + 1] + cboard[index + col - 1] +
              cboard[index + col] + cboard[index + col + 1];

    if (cboard[index] == 0) {
        if (count == 2 || count == 3) {
            return 1;
        }
        return 0;
    } else {
        if (count < 2 || count > 3) {
            return 0;
        }
        return 1;
    }

}

void update_state(int *board, int num_rows, int num_cols) {
    int end = (num_cols * num_rows) - num_cols - num_cols + 1;
    int for_end = num_cols - 2;
    int change;
    int total = num_cols * num_rows;
    int copy[total];

    for (int t = 0; t < total; t++) {
        copy[t] = board[t];
    }

    int i = num_cols + 1;
    while (i <= end) {
        for (int j = 0; j < for_end; j++) {
            change = check_around(copy, i+j, num_cols);
            board[i+j] = change;
        }
        i = i + num_cols;
    }
    return;
}
