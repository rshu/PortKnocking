#include <stdio.h>
#include <stdlib.h>

int main(void) {
  // ls -al | grep '^d'  only show directory

  FILE *pp;
  pp = popen("ls -al", "r");

  if (pp != NULL) {
    while (1) {
      char *line;
      char buf[1000];
      line = fgets(buf, sizeof buf, pp);

      if (line == NULL) {
        break;
      }

//      if (line[0] == 'd') {
      printf("%s", line);
//      }
    }

    pclose(pp);
  }

  return 0;
}
