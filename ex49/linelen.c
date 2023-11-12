#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
  int min=0, max=-1, counter=0;
  if(argc>1) // take the min if available
    min=atoi(argv[1]);
  if(argc>2) // Take the max if available
    max=atoi(argv[2]);
  char c;
  char s[60];
  for(;;){  // endlessly go until EOF
    c=getchar();
    if(c==EOF)
      break;
    if(c=='\n'){ // When the line ends
      if(counter>=min && (counter<=max || max==-1)){ // If you can print the line
        for (int i=0; i<60 && i<counter; i++) // Print up to 60 chars
          putchar(s[i]);
        if(counter>60) //if there are more than 60 chars stored, print ... 
          printf("...");
        putchar(c); // Print the new line
      }
        counter=0; // Reset the counter and start storing again
    }
    else{
      if(counter<60) // Store if you can
      s[counter]=c;
        counter++;
    }
  }
  return 0;
}
