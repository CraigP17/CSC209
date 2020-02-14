#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Write the copy function to perform exactly as strncpy does, with one
   exception: your copy function will guarantee that dest is always
   null-terminated.
   You shoud read the man page to learn how strncpy works.

  NOTE: You must write this function without using any string functions.
  The only function that should depend on string.h is memset.
 */

char *copy(char *dest, const char *src, int capacity) {
    int i = 0;
    int found = 0;
    while ((i < capacity) && (i < sizeof(dest))) {
      if (found == 1) {
        memset(&dest[i], '\0', sizeof(char));
      } else {
        memset(&dest[i], src[i], sizeof(char));
        if (src[i] == '\0') {
          found = 1;
        }
      }
      i++;
    }
    dest[capacity-1] = '\0';

    return dest;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: copy size src\n");
        exit(1);
    }
    int size = strtol(argv[1], NULL, 10);
    char *src = argv[2];

    char dest[size];
    memset(dest, 'x', size);

    copy(dest, src, size);

    printf("%s\n", dest);
    return 0;
}
