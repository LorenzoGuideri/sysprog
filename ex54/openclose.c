#include <stdio.h>
#include <string.h>
int check(char a, char s, char *S){
  for (int i=0; i<strlen(S); i+=2) {
    if (S[i]==a&&S[i+1]==s) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[])
{
  char def[] = "()[]{}"; 
  char * S= def;
  if (argc==2 ) {
    S=argv[1];
  }
  char C[1001];
  int top=0;
  char s;
  for(s=getchar(); s!=EOF;s=getchar()){
    if(s!='\n'){ 
      if(!top || check(C[top-1], s, S)){
        C[top]=s;
        top++;}
      else
       top--;
    }
    else{
      for (size_t i = 0; i < top; i++) {
        putchar(C[i]);
      }
      putchar(s);
      top=0;
    }
  }
}
