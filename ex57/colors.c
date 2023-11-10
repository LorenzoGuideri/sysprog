#include "colors.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int check(char a){
  if(isdigit(a)) 
    return a-'0';
  else if(a>='a' && a<='f')
    return a-'a'+10;
  else if(a>='A' && a<='F')
    return a-'A'+10;
  else
    return -1;
}

void string_to_color(struct color *c, const char *s){
 int index=0;
  c->red=0;
  c->green=0;
  c->blue=0;
 while( index<6 && index<strlen(s) && check(s[index])!=-1){
    switch (index) {
      case 0:
        c->red+=16*check(s[index]);
      break;
      case 1:
        c->red+=check(s[index]);
      break;
      case 2:
        c->green+=16*check(s[index]);
      break;
      case 3:
        c->green+=check(s[index]);
      break;
      case 4:
        c->blue+=16*check(s[index]);
      break;
      case 5:
        c->blue+=check(s[index]);
      break;
    }
    index++;
 }
  
}
