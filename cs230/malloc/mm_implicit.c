/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
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

void *heap_listp; /* this is the beginning of the heap */
void *heap_topp;

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
    if (extend_heap (CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }

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

    /*
    size_t prevsize = GET_SIZE(HDRP(PREV_BLKP(bp)));
    size_t nextsize = GET_SIZE(FTRP(NEXT_BLKP(bp)));
    */

    if (FTRP(PREV_BLKP(bp)) < (char *) mem_heap_lo()){
        return bp;
    }
    else if (HDRP(NEXT_BLKP(bp))> (char *) mem_heap_hi()){
        return bp;
    }
    /*
    else if(((prevsize+currsize) > (MAXHEAPSIZE))|| (prevsize < 0) || (nextsize <0) || ((nextsize+currsize) > (MAXHEAPSIZE))){
        return bp;   
    } 
    */

    else{
        size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); /* check if prev is allocated */
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); /* check if next is allocated */ 
        size_t currsize = GET_SIZE(HDRP(bp));
        if (prev_alloc && next_alloc){ /* case 1 : both are allocated */
            return bp;
        }

        else if (prev_alloc && !next_alloc) { /* case 2 : next one is free */
            currsize += GET_SIZE(HDRP(NEXT_BLKP(bp)));
                PUT(HDRP(bp), PACK(currsize, 0));
                PUT(FTRP(bp), PACK(currsize, 0));
        }
    
        else if (!prev_alloc && next_alloc){
            currsize += GET_SIZE(HDRP(PREV_BLKP(bp)));
                PUT(FTRP(bp), PACK(currsize, 0));
                PUT(HDRP(PREV_BLKP(bp)), PACK(currsize, 0));
                bp = PREV_BLKP(bp);
        }

        else {
            currsize += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))); /*why find the FTRP of the next block??? */
                PUT(HDRP(PREV_BLKP(bp)), PACK(currsize, 0));
                PUT(FTRP(NEXT_BLKP(bp)), PACK(currsize, 0));
                bp = PREV_BLKP(bp);
        }
    }
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

    if (size<= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size+(DSIZE)+(DSIZE-1))/DSIZE); /* smart */


    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
    {   
        return NULL;
    }
    place(bp,asize);
    return bp;
    
}

static void *find_fit(size_t asize)
{
    void *firstbp = (char *) heap_listp + DSIZE; /* point to the very first payload */ 
    void *currentbp = firstbp;

    size_t isFound = 0;
    size_t freeblocksize;
    size_t heapsize;
    if ((heapsize = mem_heapsize()) < asize){
        return NULL;
    }
    while ((!isFound) && (currentbp <  mem_heap_hi())&& (currentbp > mem_heap_lo())){
        if (!GET_ALLOC(HDRP(currentbp))){
            freeblocksize = GET_SIZE(HDRP(currentbp));
            if (freeblocksize >= asize){
                isFound = 1;
                return (void *)currentbp;
            }
            else{
                currentbp = NEXT_BLKP(currentbp);
            }
        }
        else {
        /*if not free update firstbp to next blockp */
            currentbp = NEXT_BLKP(currentbp);
        }
    }
    /*
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) >0 ; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    */
    return NULL;
}

static void place(void *bp, size_t asize){
    size_t currentblocksize = GET_SIZE(HDRP(bp));
    /* currentblocksize includes header, footer */
    if ((currentblocksize - asize)>= (2*DSIZE)){
        //void *splitbp = bp + asize; 
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(currentblocksize - asize, 0));
        PUT(FTRP(bp), PACK(currentblocksize - asize, 0));
        /*
        PUT(bp + asize - DSIZE, PACK(asize, 1));
        PUT(splitbp-WSIZE, PACK((currentblocksize - asize), 0));
        printf("splitbp, currentblocksize , asize %p, %u, %d\n", splitbp, currentblocksize, asize);
        PUT(splitbp+(currentblocksize-asize), PACK((currentblocksize - asize ), 0));
    printf("dadf\n");
    */
    }
    else {

        PUT(HDRP(bp), PACK(currentblocksize, 1));
        PUT(FTRP(bp), PACK(currentblocksize, 1));
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
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    /*  1. ptr ==  null and size >0: equivalent to malloc(size) + copy memory 
        2. ptr ==  null and size ==0 : ??????
        3. ptr !=  null and size == 0: mm_free(ptr)
        4. ptr !=  null and size >0 (most cases) 
     */
    if (ptr == NULL && size ==0){
        return NULL;
    }

    else if (size ==0 && ptr != NULL)
    {
        mm_free(ptr);
        return ptr;
    }
    else{
            //printf("valid newptr\n");
            newptr =  mm_malloc(size);
           // printf("newptr is %p\n", newptr);

        if (newptr != NULL) {
            copySize = (*(unsigned int *)((char *)oldptr - WSIZE) & ~0x7)- DSIZE;
            //copySize = GET_SIZE(HDRP(ptr)) - DSIZE;
            /*
            if ((void *) ((size_t) newptr + (size_t) copySize) > mem_heap_hi())
            {
                printf("overflow\n");
               return NULL; 
            }
            */
            if (size < copySize)
            {
                copySize = size;
            }           
            memcpy(newptr, oldptr, copySize);
            if(oldptr != newptr)
            {
                mm_free(oldptr);

               }
        } else{
            printf("null pointer \n");
        }
    }

    return newptr;
}


int mm_check(void)
{
    // is every block in the free list marked free?
    // are there contiguous free blocks that has not been coalesced
    // is every free block actually in the free list?
    // do pointers in the free list point to valid free blocks?
    // do any allocated free blocks overlap?
    // do pointers in heap block point to valid heap addresses?
    return 0;

}













