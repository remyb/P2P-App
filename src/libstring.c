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

  char * rv = malloc( ((end-begin)+2)*(sizeof(char)) );
  memset(rv, 0, ((end-begin)+2)*(sizeof(char)) );
  int loop = 0;
  int temp = begin;

  for(loop = 0; loop <= end-begin; loop++,temp++) {
    rv[loop] = string[temp];
  }
  rv[loop] = 0;

  return rv;
}

int validFilename(char * string) {
  int period = 1;
  int loop = 0;

  if(strlen(string) > 20) {
    return 0;
  }

  for(loop = 0; loop < strlen(string); loop++) {
    if(string[loop] < 97 || string[loop] > 122) {
      if(string[loop] == 46 && period < 1)
        return 0;
      if(string[loop] == 46 && period > 0) {
        period--;
        continue;
      }
      if(string[loop] > 47 && string[loop] < 58) 
	continue;
      if(string[loop] != 95)
        return 0;
    }
  }
  return 1;
}
