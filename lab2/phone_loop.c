#include <stdio.h>
#include <stdlib.h>

int main() {
  char phone[11];
  int index;
  int scan = 0;
  int wrong = 0;
  scanf("%s %d", phone, &scan);

  while(scan != EOF) {
    if (index == -1) {
        printf("%s\n", phone);
    } else if ((0 <= index)&&(index <= 9)) {
        printf("%c\n", phone[index]);
    } else {
      wrong = 1;
      printf("ERROR\n");
    }
    scan = scanf("%d", &index);
  }

  if (wrong) {
    return 1;
  }
  return 0;
}
