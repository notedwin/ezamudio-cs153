#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
    
void usageCmd(){//prints usage of the .c file and how the flags relate
printf("Usage: ./csim [-h] [-v] -s <s> -E <E> -b <b> -t <tracefile>\n");
printf("-s: number of set index(2^s sets)\n");
printf("-E: number of lines per set\n");
printf("-b: number of block offset bits\n");
printf("-t: trace file name\n");}

typedef struct{
    int s;
    int S;
    int b;	
    int B;
    int E;	
    int hits;
    int misses;
    int evictions;
    int verbosity;
} cache_t;

typedef struct {
    int valid;
    unsigned long long tag;
    char *block;
    int counter;
}set_line;

typedef struct {
    set_line *lines;
}cache_set;

typedef struct {
    cache_set *sets;
}cache_sets;

cache_sets create(long long S, int E){
    cache_sets new_cache;	
    cache_set set;
    set_line line;
    new_cache.sets = (cache_set *) malloc(sizeof(cache_set) * S);

    for (int i = 0; i < S; i++){
        set.lines =(set_line *) malloc(sizeof(set_line) * E);
        new_cache.sets[i] = set;
        for (int j = 0; j < E; j++){
            line.valid = 0; 
            line.tag = 0; 
            set.lines[j] = line;
            line.counter = 0;
        }} return new_cache;}

int checkHit(set_line line,unsigned long long tag){
    if(line.valid){
        if(line.tag == tag){
           return 1;}}return 0;}
int checkFull(cache_set set, cache_t prop){
    for(int i = 0; i<prop.E; i++){
        if(set.lines[i].valid == 0){
            return 1;}}return 0;}
int findIndex(cache_set set, cache_t prop){
    for(int i = 0; i<prop.E; i++){
        if(set.lines[i].valid == 0){
            return i;}}return -1;}
int findEvict(cache_set set, cache_t prop){
    int min = set.lines[0].counter;
    int index = 0;
    for(int i = 0; i < prop.E ; i++){
        if(min>set.lines[i].counter){
            index = i;
            min = set.lines[i].counter;
        }}return index;}
int findMax(cache_set set, cache_t prop){
    int max = set.lines[0].counter;
    int index = 0;
    for(int i = 0; i < prop.E ; i++){
        if(set.lines[i].counter>max){
            index = i;
            max = set.lines[i].counter;
        }}return index;}

cache_t simul(cache_sets curr_cache,cache_t prop,unsigned long long address){
    int tagSize = 64-(prop.b + prop.s);
    unsigned long long tag = address >> (prop.s + prop.b);
    unsigned long long temp = address << (tagSize);
    unsigned long long setIndex = temp >> (tagSize + prop.b);
    cache_set set = curr_cache.sets[setIndex];
    
    int hit = 0;
    for (int i = 0; i<prop.E; i++){
        set_line currentLine = set.lines[i];
        if(checkHit(currentLine, tag) == 1){
            prop.hits+=1;
            int max = 0;
            hit = 1;
            max = findMax(set, prop);
            curr_cache.sets[setIndex].lines[i].counter = curr_cache.sets[setIndex].lines[max].counter+1;
        }}
    if(hit == 0 && checkFull(set, prop) == 1){
        prop.misses+=1;
        int index = 0;
        index = findIndex(set, prop);
        set.lines[index].tag = tag;
        set.lines[index].valid = 1;
        int max = 0;
        max = findMax(set, prop);
        curr_cache.sets[setIndex].lines[index].counter = curr_cache.sets[setIndex].lines[max].counter+1;
    }else if(hit == 0){
        prop.misses+=1;
        prop.evictions+=1;
        int evictIndex = findEvict(set, prop);
        set.lines[evictIndex].tag = tag;
        int max = findMax(set, prop);
        curr_cache.sets[setIndex].lines[evictIndex].counter = curr_cache.sets[setIndex].lines[max].counter+1;
    }
    return prop;
}
void clear_cache(cache_sets curr_cache,cache_t prop){
	
}

int main(int argc, char** argv)
{
    cache_sets curr_cache;
    cache_t prop;
    char opt;
    char* file_nam;
    prop.verbosity = 0;
    
    while((opt=getopt(argc,argv,"s:E:b:t:vh")) != -1)//this loop sets all the values for the Cache
    {
        switch(opt){
	case 's':
            prop.s = atoi(optarg);
            break;
        case 'E':
            prop.E = atoi(optarg);
            break;
        case 'b':
            prop.b = atoi(optarg);
            break;
        case 't':
            file_nam = optarg;
            break;
	case 'v':
	    prop.verbosity=1;
	    break;
	case 'h':
	   usageCmd(); 
	   break;
        default:
            usageCmd();
            exit(-1);
        }
    }
    prop.S = pow(2.0,prop.s);
    prop.B = pow(2.0,prop.b);
    curr_cache = create(prop.S, prop.E);
    prop.misses = 0;
    prop.hits = 0;
    prop.evictions = 0;
    //start using trace file
    FILE *trace;
    char command;
    unsigned long long address;
    int size;
    trace = fopen(file_nam, "r");
    while(fscanf(trace, " %c %llx,%d", &command, &address, &size) == 3){
        switch(command){
            case 'I':
                break;
            case 'L':
                prop = simul(curr_cache, prop, address);
                break;
            case 'S':
                prop = simul(curr_cache, prop, address);
                break;
            case 'M':
                prop = simul(curr_cache, prop, address);
                prop = simul(curr_cache, prop, address);	
                break;
            default:
                break;}}
    printSummary(prop.hits, prop.misses, prop.evictions);
   // clear_cache(curr_cache,prop);
    fclose(trace);
    return 0;
}

