#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {    
if (argc < 3) {
    printf("Usage: %s STRING1 STRING2\n", argv[0]);
    printf("       %s -d STRING\n", argv[0]);
    exit(1);}

    int i,c;

  if(strcmp(argv[1],"-d")==0){
    int to_delete[256];
    for(i=0;i<256;i++){
      to_delete[i]=0;}
    for(i=0;i<strlen(argv[2]);i++){
        to_delete[(int)argv[2][i]]=i+1;}
    while(1){
      c = fgetc(stdin);
      if(c == EOF){break;}
      if(to_delete[c]==0){
        fwrite(&c,1,1,stdout);}
      }exit(0);}


  if(strlen(argv[1])!=strlen(argv[2])){
    printf("STRING1 and STRING2 must have the same length\n");
    exit(1);}

    i=0;c=0;
    int replacer[256];
    for(i=0;i<256;i++) replacer[i]=i;//creates a map of all char
    for(i=0;i<strlen(argv[1]);i++){
     replacer[(int)argv[1][i]]=argv[2][i];}//swap values for the ones we want
    while(1){
      c  = fgetc(stdin);//gets 1 char
      if(c==EOF){break;}//if eof then end
      if(c!=replacer[c]){fwrite(&replacer[c],1,1,stdout);} 
      if(c==replacer[c]){fwrite(&c,1,1,stdout);}}

exit(0);
return 0;}
