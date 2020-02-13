#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s FILENAME\n", argv[0]);
    exit(1);
  }
  FILE *from = fopen(argv[1],"r");
  int nread=1;
  int c;
  int a=fgetc(from);
  
  while(1){
  c = fgetc(from);
  if(c!=a){
  fwrite(&nread,sizeof(nread),1,stdout);
  fwrite(&a,1,1,stdout);
  a=c;
  nread=1;
  }else{
  nread++;
   }
  if(c==EOF){
  break;
   }
  }
  exit(0);
  return 0;
}
