#include "libstring.h"

int containsChar(char * string, char c) {
  int stringLength = 0;
  int loop = 0;
  stringLength = strlen(string);

  for(loop = 0; loop < stringLength; loop++) {
    if(string[loop] == c)
      return loop;
  }

  return -1;
}

char * getSubstring(char * string, int begin, int end) {
  int stringLength = 0;
  stringLength = strlen(string);

  char * rv = malloc( ((end-begin)+1)*(sizeof(char)) );
  int loop = 0;
  int temp = begin;

  for(loop = 0; loop <= end-begin; loop++,temp++) {
    rv[loop] = string[temp];
  }
  rv[++loop] = 0;

  return rv;
}
