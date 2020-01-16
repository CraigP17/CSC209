#include <stdio.h>
#include <stdlib.h>

int main() {
  char phone[11];
  int index;
  int scan = 0;
  scanf("%s", phone);

  do {
    scan = scanf("%d", &index);
    if (index == -1) {
        printf("%s\n", phone);
    } else if ((0 <= index)&&(index <= 9)) {
        printf("%c\n", phone[index]);
    } else {
      printf("ERROR\n");
      return 1;
    }
  } while(scan != EOF);

  return 0;
}
