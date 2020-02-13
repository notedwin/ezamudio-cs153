#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s FILENAME\n", argv[0]);
    exit(1);
  }
  
  FILE *from = fopen(argv[1],"r");
  while(1){
    int i=0;
    char c=0;
    if(feof(from))break;
    fread(&i,sizeof(i),1,from);
    if(feof(from))break;
    fread(&c,sizeof(c),1,from);

    for(int z=0;z<i;z++){
      fwrite(&c,1,1,stdout);
    }
  }
  exit(0);
	
  return 0;
}
