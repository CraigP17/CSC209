#include <stdio.h>

#include "benford_helpers.h"

int count_digits(int num) {
    int digit = 0;
    int rep = num;
    if (rep == 0) {
      digit = 1;
    } else {
      while (rep != 0) {
        rep = rep / BASE;
        digit++;
      }
    }
    return digit;
}

int get_ith_from_right(int num, int i) {
    int digit = 0;
    int count = count_digits(num);
    int base_arr[count];
    int j = count - 1;
    while (num != 0) {
      base_arr[j] = num % BASE;
      num = num / BASE;
      j--;
    }
    digit = base_arr[count - i - 1];
    return digit;
}

int get_ith_from_left(int num, int i) {
    int digit = 0;
    int count = count_digits(num);
    int base_arr[count];
    int j = count - 1;
    while (num != 0) {
      base_arr[j] = num % BASE;
      num = num / BASE;
      j--;
    }
    digit = base_arr[i];
    return digit;
}

void add_to_tally(int num, int i, int *tally) {
    int tally_dig;
    tally_dig = get_ith_from_left(num, i);
    tally[tally_dig] = tally[tally_dig] + 1;
    return;
}
