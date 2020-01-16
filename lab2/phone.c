#include <stdio.h>
#include <stdlib.h>

int main() {
    char phone[11];
    int index;
    scanf("%s %d", phone, &index);
    if (index == -1) {
        printf("%s\n", phone);
        return 0;
    } else if ((0 <= index)&&(index <= 9)) {
        printf("%c\n", phone[index]);
        return 0;
    } else {
      printf("ERROR\n");
      return 1;
    }
}
