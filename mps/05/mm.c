#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define BHEADER_SIZE ALIGN(sizeof(bHeader))
#define STRIPPED_HEADER ALIGN(sizeof(size_t))
#define BFOOTER_SIZE ALIGN(sizeof(bFooter))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define BINS 10

typedef struct header bHeader; 
typedef struct footer bFooter;
bHeader * find_fit(size_t size);

struct header{
	size_t size;
	bHeader * next;
	bHeader * prev;};
struct footer{bHeader * head;};
struct header heads[BINS];
int cutoff[BINS] = {17,69,113,129,449,1621,4073,4096,8191,INT_MAX};

int getIndex(size_t size){
int index;
   for(index = 0; index<BINS; ++index){
      if(size< cutoff[index])
         break;
   }
   return index;
}

int count;
/* 
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
  count = 0;
   bHeader *p = mem_sbrk(BHEADER_SIZE+BFOOTER_SIZE);
   p->next=p;
   p->prev=p;
   p->size=BHEADER_SIZE+BFOOTER_SIZE+1;
   bFooter *q=(bFooter *)((char *)p+BHEADER_SIZE);
   q->head=p;
   int i;
   for(i = 0; i<BINS;++i){
      heads[i].next=&heads[i];
      heads[i].prev=&heads[i];
      heads[i].size=BHEADER_SIZE+BFOOTER_SIZE+1;
   }
  return 0;
}
/* mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{
   size_t newsize = ALIGN(size + STRIPPED_HEADER+ BFOOTER_SIZE);
   bHeader *p;
   if(count > 0 && (p=find_fit(newsize))!=NULL){
      if(((p->size)&~1)-newsize>=7+BHEADER_SIZE+BFOOTER_SIZE){
         size_t oldsize = p->size;
         p->size = newsize;
         ((bFooter *)((char *)p+((p->size)&~1)-BFOOTER_SIZE))->head = p;
         bHeader *split=(bHeader *)((char *)p + (p->size & ~1));
         split->size=oldsize-p->size; 
         ((bFooter *)((char *)split+((split->size)&~1)-BFOOTER_SIZE))->head = split;
         int index = getIndex(split->size-STRIPPED_HEADER-BFOOTER_SIZE);
         split->next = heads[index].next;
         heads[index].next=split;
         split->next->prev=split;
         split->prev=&heads[index];
         ++count;
      }
      if(count>0){
         --count;
      }
      p->prev->next=p->next;
      p->next->prev=p->prev;
      p->size|= 1;
   }else{
      bHeader * last=(((bFooter *)((char *)mem_heap_hi()+1-BFOOTER_SIZE))->head);
      if((count>0)&&(!(last->size &1))){
         --count;
         mem_sbrk(newsize-last->size);
         last->size = newsize | 1;
         ((bFooter *)((char *)last+((last->size)&~1)-BFOOTER_SIZE))->head = last;
         last->prev->next=last->next;
         last->next->prev=last->prev;
         p = last;
      } else {
         p = mem_sbrk(newsize);
         if ((long)p == -1){
            return NULL;
         }
         else {
            p->size=newsize | 1;
            bFooter *q=(bFooter *)((char *)p+((p->size)&~1)-BFOOTER_SIZE);
            q->head=p;
         }
      }
   }
   return (void *)((char *)p+sizeof(size_t));
}

bHeader * find_fit(size_t size){
   int index = getIndex(size);
   bHeader * p ;
   for(; index<BINS; ++index){
      for(p = heads[index].next; (p != &heads[index])&&(p->size<size+STRIPPED_HEADER+BFOOTER_SIZE);p=p->next);
      if(p != &heads[index])
         return p;
   }
   return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
   ++count;
   bHeader *p = (bHeader *)((char *)ptr - STRIPPED_HEADER);
   bHeader *prev = ((bFooter *)((char *)p - BFOOTER_SIZE))->head;
   p->size &= ~1;
   if(!((prev->size)&1)){
      prev->prev->next=prev->next;
      prev->next->prev=prev->prev;
      prev->size+=p->size;
      ((bFooter *)((char *)p+((p->size)&~1)-BFOOTER_SIZE))->head = prev;
      p = prev;
      --count;
   }
   bHeader *next=(bHeader *)((char *)p + (p->size &= ~1));
   if(((void *)((char *)p+p->size) <= mem_heap_hi()) && !((next->size)&1)){
      p->size+=next->size;
      ((bFooter *)((char *)p+((p->size)&~1)-BFOOTER_SIZE))->head = p;
      next->prev->next=next->next;
      next->next->prev=next->prev;
      --count;
   }
   bHeader * front = &heads[getIndex(p->size-STRIPPED_HEADER-BFOOTER_SIZE)];
   p->prev=front;
   p->next=front->next;
   front->next->prev = p;
   front->next=p;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
size_t newSize = ALIGN(size + STRIPPED_HEADER+ BFOOTER_SIZE);
   bHeader *p = (bHeader *)((char *)ptr - sizeof(size_t));
   bHeader *next=(bHeader *)((char *)p + (p->size &= ~1));
   if((p->size&~1)>newSize){	
      return ptr;
   }
   if(((void *)((char *)p+p->size) <= mem_heap_hi()) && !((next->size)&1) && (next->size + (p->size&~1) > newSize)){
      p->size=(p->size+next->size)|1;
      ((bFooter *)((char *)p+((p->size)&~1)-BFOOTER_SIZE))->head = p;
      next->prev->next=next->next;
      next->next->prev=next->prev;
      return ptr;
   }
   if(((void *)((char *)p+p->size) >= mem_heap_hi())){
      mem_sbrk(newSize-p->size);
      p->size=newSize;
      ((bFooter *)((char *)p+((p->size)&~1)-BFOOTER_SIZE))->head = p;
      return ptr;
   }
   void * new = mm_malloc(size);
   memcpy(new,ptr,p->size-STRIPPED_HEADER-BFOOTER_SIZE);
   mm_free(ptr);
   return new;

}
