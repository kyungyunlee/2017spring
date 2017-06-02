/*
 * EXPLICIT FREE LIST IMPLEMENTATION
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))  /* one word size */

/* STARTS HERE */

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t size);
static void place(void *bp, size_t asize);
static void addToRoot(void *bp);
static void removeFree(void *bp);
int mm_check(void);

void *heap_listp; /* this is the beginning of the heap */
void *heap_topp;

void *root_freelist;
//int freelistcounter = 0;
int countrealloc = 0;

#define WSIZE 4
#define DSIZE 8 /* double word */
#define CHUNKSIZE (1<<12) /* extend heap by this size...isn't it too much? since max heap size is 20*2^20*/
#define MAX(x,y) ((x) > (y)? (x) : (y))

/* for header and footer -- combine size and allocated bit*/
#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *) (p)) /* get the word at address p */
#define PUT(p, val) (*(unsigned int *) (p) = (val)) /* put new val at address p */

/* get size of the block at address p */
#define GET_SIZE(p) (GET(p) & ~0x7) /* mask last three bits */
/* check if block at adderss p is free or allocated */
#define GET_ALLOC(p) (GET(p) & 0x1) /* last bit is alloc bit */

/* bp points to the first payload byte */
#define HDRP(bp) ((char *) (bp) - WSIZE)
/* why is header WSIZE and footer DSIZE ??????  */
#define FTRP(bp) ((char *) (bp) + GET_SIZE(HDRP(bp)) -(WSIZE*2))

#define NEXT_BLKP(bp) ((char *) (bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *) (bp) - GET_SIZE(((char *)(bp) - (WSIZE*2))))

// for EXPLICIT LIST 
// looks disgusting..............
#define NEXT_FREEP(bp) ((void *)*(size_t *)(bp))
#define PREV_FREEP(bp) ((void *)*(size_t *)((char *)(bp) + WSIZE))
#define PUT_ADDR(p, address) (*(char **)(p) = (char *)(address))
#define VALID(address) (((void *)(address) > (mem_heap_lo())) && ((void *)(address) < mem_heap_hi()) ? 1 : 0)
#define ADD_W(p) ((void *)((char *) (p) + WSIZE))
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *) -1){
        return -1;
    }
    /* if no error, heap_listp = address of the new heap area */
    heap_listp = mem_heap_lo();
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0,1));
    heap_listp += (2*WSIZE);
    
    root_freelist = NULL;
    if (extend_heap (CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }
    //mm_check();
    return 0;
}
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    size = (words %2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1));
    return coalesce(bp);
}

static void *coalesce(void *bp){

        size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); /* check if prev is allocated */
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); /* check if next is allocated */ 
        size_t currsize = GET_SIZE(HDRP(bp));
        
        if (!VALID(PREV_BLKP(bp))) { // it is the root so just consider next block
            prev_alloc = 1;
        }
        if (!VALID(NEXT_BLKP(bp))){ // it is at the end so just consider prev block
            next_alloc = 1;
        }

        if (prev_alloc && !next_alloc) { // case 2 : next one is free 
                removeFree(NEXT_BLKP(bp));
                currsize += GET_SIZE(HDRP(NEXT_BLKP(bp)));
                PUT(HDRP(bp), PACK(currsize, 0));
                PUT(FTRP(bp), PACK(currsize, 0));
        }
    
        else if (!prev_alloc && next_alloc){ // case 3: prev one is free
                removeFree(PREV_BLKP(bp));
                currsize += GET_SIZE(HDRP(PREV_BLKP(bp)));
                PUT(FTRP(bp), PACK(currsize, 0));
                PUT(HDRP(PREV_BLKP(bp)), PACK(currsize, 0));
                bp = PREV_BLKP(bp);
        }
        else if (!prev_alloc && !next_alloc){
                removeFree(PREV_BLKP(bp));
                removeFree(NEXT_BLKP(bp));
                currsize += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
                PUT(HDRP(PREV_BLKP(bp)), PACK(currsize, 0));
                PUT(FTRP(NEXT_BLKP(bp)), PACK(currsize, 0));
                bp = PREV_BLKP(bp);
        }
        addToRoot(bp);
    return bp;

}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;
    if(size ==0)
        return NULL;

    if (size > DSIZE)
        asize = DSIZE * ((size+(DSIZE)+(DSIZE-1))/DSIZE); /* smart */
    else
        asize = 2*DSIZE;
    
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        //mm_check();
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
    {   
        return NULL;
    }
    place(bp,asize);
    //mm_check();
    return bp;
    
}

static void *find_fit(size_t asize)
{
    /* EXPLICIT LIST */
    void *currentbp = root_freelist;

    size_t isFound = 0;
    size_t freeblocksize;
    while ((!isFound) && VALID(currentbp)){
        freeblocksize = GET_SIZE(HDRP(currentbp));
        if (freeblocksize >= asize){
            isFound =1;
            return (void *) currentbp;
        }
        else{
            currentbp =(void *) NEXT_FREEP(currentbp);
        }
     }
    return NULL;
}

static void place(void *bp, size_t asize){

    size_t currentblocksize = GET_SIZE(HDRP(bp));
    void *nextfreep = (void *)NEXT_FREEP(bp);
    void *prevfreep = (void *)PREV_FREEP(bp);
    if ((currentblocksize - asize)>= (2*DSIZE)){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(currentblocksize - asize, 0));
        PUT(FTRP(bp), PACK(currentblocksize - asize, 0));
        if (VALID(prevfreep) && VALID(nextfreep)){
            PUT_ADDR(bp, nextfreep);
            PUT_ADDR(ADD_W(bp), prevfreep);
            PUT_ADDR(prevfreep, bp);
            PUT_ADDR(ADD_W(nextfreep), bp);
        }
        else if (VALID(nextfreep)){ //prev is not valid = bp is root
            PUT_ADDR(bp, nextfreep);
            PUT_ADDR(ADD_W(bp), NULL);
            PUT_ADDR(ADD_W(nextfreep), bp);
            root_freelist = bp;
        }
        else if(VALID(prevfreep)){
            PUT_ADDR(bp, NULL);
            PUT_ADDR(ADD_W(bp), prevfreep);
            PUT_ADDR(prevfreep, bp);
        }
        else{
            root_freelist = bp;
            PUT_ADDR(bp, NULL);
            PUT_ADDR(ADD_W(bp) , NULL);
        }

    }
    else {
        PUT(HDRP(bp), PACK(currentblocksize, 1));
        PUT(FTRP(bp), PACK(currentblocksize, 1));
        
        removeFree(bp);
    }

}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
 
    coalesce(ptr);
    //mm_check();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    //mm_check();
    void *oldptr = ptr;
    void *newptr;
    size_t currentblocksize = GET_SIZE(HDRP(ptr));

    /*
     Realloc notes 
     case 1 ( input size == 0 ) : mm_free
     case 2 ( ptr == NULL) : mm_malloc
     case 3 ( both ptr and input are valid ) : 
        if size < currentblocksize, we dont have to do anything. just keep original.
        if size > currentblocksize, 
            3-1 if next block is free, coalesce with next block and reallocate
            3-2 else just mm_malloc
            *******memcpy DOESNT WORK if the block overlaps...so there is no point of coalescing with the previous free block.... :(
     Also, for realloc-bal.rep, count number of 3-2 operations (which is one that uses mm_malloc and therefore slower), if there is repeated number of "failed" reallocs like in realloc-bal, then call double-sized mallocs.
     */


    if (size == 0){
        mm_free(oldptr);
        return NULL;
    }

    if (ptr == NULL){
        newptr = mm_malloc(size);
        return newptr;
    }
    
    size_t asize;
    if (size > DSIZE)
       asize = DSIZE * ((size + (DSIZE)+(DSIZE-1))/DSIZE);
    else
        asize = 2*DSIZE;
    if (asize < currentblocksize){ // if new size is smaller then free the remaining
        return oldptr;
    }
    else {
        size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(oldptr)));
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(oldptr)));
        size_t combinedsize;
        if (prev_alloc && !next_alloc){ // combine with the next block 
            combinedsize = currentblocksize + GET_SIZE(HDRP(NEXT_BLKP(oldptr)));
            if (combinedsize > asize) // then combining works 
            {

               removeFree(NEXT_BLKP(oldptr)); //WHAT THE HECK!!
                PUT(HDRP(oldptr), PACK(currentblocksize, 0));
                PUT(FTRP(oldptr), PACK(currentblocksize, 0));
                coalesce(oldptr);
                // don't need to copy memory, just place
                place(oldptr, asize);
                return oldptr;

            }
            
        }

        // for realloc-bal.rep ....WHAT THIS SOLVED EVERYTHING.......
        // wow how does just this increase my score from 44 to 82...........
        if (countrealloc >= 30)
        {
            newptr = mm_malloc(size*2);
        }
        else {

            newptr = mm_malloc(size);
        }
            memcpy(newptr, oldptr, currentblocksize - DSIZE);
            mm_free(oldptr);
            countrealloc += 1;
            return newptr;

    }

}

int mm_check(void)
{
    void * i;
    int freeblockcounter1 = 0;
    /*
    for (i = root_freelist; VALID(i); i = NEXT_FREEP(i)){
        freeblockcounter1 +=1;
       printf("freeblock at %p prevfree %p nextfree %p\n", i, (void *)PREV_FREEP(i), (void *)NEXT_FREEP(i));
    }
    */
    for (i = root_freelist; VALID(i); i = NEXT_FREEP(i)){
        if (GET_ALLOC(HDRP(i)) != 0){
            printf("root is %p\n", root_freelist);
            printf("prev and next is %p, %p\n", PREV_FREEP(i), NEXT_FREEP(i));
            printf("Free list check : Block at %p is not marked free\n", i);
        }
    // are there contiguous free blocks that has not been coalesced
        if (VALID(PREV_BLKP(i))){
            if (GET_ALLOC(HDRP(PREV_BLKP(i))) == 0) {
                printf("Coalsece check : Not coalesced with previous block at %p\n", PREV_BLKP(i));
            }
        }
    // do pointers in the free list point to valid free blocks?
        void * prevptr = (void *) PREV_FREEP(i);
        void * nextptr = (void *)NEXT_FREEP(i);
           if (GET_ALLOC(HDRP(prevptr)) != 0){
            printf("Previous free block is not free %p\n", prevptr);
           }
           if (GET_ALLOC(HDRP(nextptr)) != 0){
            printf("Next free block is not free %p\n", nextptr);
           }
    }
    
    int freeblockcounter2 = 0;
    // is every free block actually in the free list?
    printf("heap listp is %p root is %p\n", heap_listp, root_freelist);
    for ( i = heap_listp; VALID(i); i = NEXT_BLKP(i)){
        if (GET_ALLOC(HDRP(i)) == 0){
        //    printf("in entire list, free block found at %p\n", i);
            freeblockcounter2 += 1;
        }
    }
   if (freeblockcounter1 != freeblockcounter2){
        printf("Free list vs actual free blocks: freelist %d, entire list %d\n", freeblockcounter1, freeblockcounter2);
   }

    return 1;

}
static void addToRoot (void *bp)
{
    PUT_ADDR(bp, root_freelist);
    PUT_ADDR(ADD_W(bp), NULL);
    if (root_freelist != NULL)
        PUT_ADDR(ADD_W(root_freelist), bp);
    root_freelist = bp; 
}

static void removeFree (void *bp)
{
    void *prevfree = (void *)PREV_FREEP(bp);
    void *nextfree = (void *)NEXT_FREEP(bp);
    //PUT_ADDR(bp, NULL);
    //PUT_ADDR(ADD_W(bp), NULL);
    if (!VALID(prevfree) && !VALID(nextfree)){
        root_freelist = NULL;
    }
    else if (!VALID(prevfree)){
        PUT_ADDR(ADD_W(nextfree), NULL);
        root_freelist = nextfree;
    }
    else if (!VALID(nextfree)){
        PUT_ADDR(prevfree, NULL);
    }
    else {
        PUT_ADDR(prevfree, nextfree);
        PUT_ADDR(ADD_W(nextfree), prevfree);
        
    }


}


