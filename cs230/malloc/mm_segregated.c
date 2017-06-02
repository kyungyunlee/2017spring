/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 

 Steps for getting this far :
 1. implicit free list with text book code
    - util 44/60 + thru 0/40

 2. explicit free list with doubly linked list using spaces within the free block 
    - without realloc optimization : util 44/60 + thru 4/40...
    - by adding like 3 lines.. : throughput increased to 38/40 (spent like 5 hours debugging this.....and it just needed an optimization :( )

 3. segregated free list of 15 groups -> 12 groups(since min block size is 16 bytes, maybe groups for too small of a block is not necessary??? idk...)
    - util 44/60 + thru 40/40
    - since i had time, i wanted to try out segregated free list as a challenge and it was actually pretty easy, since it is literally based off of explicit free list.


IMPLEMENTED USING SEGREGATED FREE LIST , which I built on top of my explicit free list implementation

- Made 12 groups of sizes from 16, 32, 64, ..., 2^15, so group 0 contains free blocks of size <= 16, group 1 contains size <=32, and the last group contains blocks that have size greater than 2^14.
- Free blocks contain 4 byte header and footer and immediately following the header, there is an address to the next free block and address to the previous free block.

HEADER
NEXT FREE BLK
PREV FREE BLK
- 
FOOTER

- Allocated blocks only contain header and footer.
- When mm_malloc gets called, find_fit function will be first executed, where the search for the free block in the correct size group will occur. If the correct size group does not have a free block, then it will search the next size group. If not found in the free list, then the heap gets extended. 
- When placing the new allocated size block on the chosen free block, there might be a need to split the free block. If it is splitted, the splitted free block will be added to a new size group. 
- When freeing a block in cases such as mm_free, extend_heap and mm_realloc, then the new free block will be checked for whether it needs to be coalesced with adjacent blocks if they are free. The coalesced block will have a larger size and will find a new size group to be linked to.

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
    "why are you doing this to me mr malloc",
    /* First member's full name */
    "Kyung Yun Lee",
    /* First member's email address */
    "kyungyunlee2393@kaist.ac.kr",
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
static void removeFree(void *bp, int sizegroup);
int mm_check(void);
int findGroup (size_t size);

void *heap_listp; /* this is the beginning of the heap */
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
#define VALID(address) (((void *)(address) > (mem_heap_lo() + 48)) && ((void *)(address) < mem_heap_hi()) ? 1 : 0)
#define ADD_W(p) ((void *)((char *) (p) + WSIZE))

//added for SEGLIST
#define NUM_OF_GROUPS 12
//groupsptr is a pointer to the groups list.
void *groupsptr;
//size_t *groups[NUM_OF_GROUPS];


/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*WSIZE + 4*NUM_OF_GROUPS)) == (void *) -1){
        return -1;
    }
    int i;
    groupsptr = heap_listp;
    for (i = 0 ; i <NUM_OF_GROUPS; i++ ) {
        //POINTERS ARE SO DIFFICULT TO UNDERSTAND......WAS NULL A POINTER??...
        *(unsigned int *)(groupsptr + (4*i)) = (unsigned int)NULL;

    }
    /* if no error, heap_listp = address of the new heap area */
    heap_listp = mem_heap_lo()+48;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));
    PUT(heap_listp + (3*WSIZE), PACK(0,1));
    heap_listp += (2*WSIZE);
    
    if (extend_heap (CHUNKSIZE/WSIZE) == NULL){
        return -1;
    }
   /// mm_check();
    return 0;
}

/*
 * extend_heap - extend the heap by words size, coalesce if possible and add to the free list
 */
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


/*
 * coalese - check if previous and next blocks are free. If any of them or both of them are free, combine them. Add to the appropriate size group's free list.
 */
static void *coalesce(void *bp){

        size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // check if prev is allocated 
        size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // check if next is allocated  
        size_t currsize = GET_SIZE(HDRP(bp));
        
        if (!VALID(PREV_BLKP(bp))) { // it is the root so just consider next block
            prev_alloc = 1;
        }
        if (!VALID(NEXT_BLKP(bp))){ // it is at the end so just consider prev block
            next_alloc = 1;
        }

        if (prev_alloc && !next_alloc) { // case 2 : next one is free 
                removeFree(NEXT_BLKP(bp), findGroup(GET_SIZE(HDRP(NEXT_BLKP(bp)))));
            currsize += GET_SIZE(HDRP(NEXT_BLKP(bp)));
                PUT(HDRP(bp), PACK(currsize, 0));
                PUT(FTRP(bp), PACK(currsize, 0));
                //addToRoot(bp);
        }
    
        else if (!prev_alloc && next_alloc){ // case 3: prev one is free
                removeFree(PREV_BLKP(bp), findGroup(GET_SIZE(HDRP(PREV_BLKP(bp)))));
            currsize += GET_SIZE(HDRP(PREV_BLKP(bp)));
                PUT(FTRP(bp), PACK(currsize, 0));
                PUT(HDRP(PREV_BLKP(bp)), PACK(currsize, 0));
                //addToRoot(PREV_BLKP(bp));
                bp = PREV_BLKP(bp);
        }
        else if (!prev_alloc && !next_alloc){
            removeFree(PREV_BLKP(bp), findGroup(GET_SIZE(HDRP(PREV_BLKP(bp)))));
            removeFree(NEXT_BLKP(bp), findGroup(GET_SIZE(HDRP(NEXT_BLKP(bp)))));
            currsize += GET_SIZE(FTRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))); //why find the FTRP of the next block??? 
                PUT(HDRP(PREV_BLKP(bp)), PACK(currsize, 0));
                PUT(FTRP(NEXT_BLKP(bp)), PACK(currsize, 0));
                //addToRoot(PREV_BLKP(bp));
                bp = PREV_BLKP(bp);
        }
        addToRoot(bp);
    //}
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

/*
 * find_fit - find the correct size group and check if there is a free block in the correct size group. If not found, check the next group, and if not found in the entire free list, then return NULL. 
 */
static void *find_fit(size_t asize)
{
    size_t sizegroup = findGroup(asize);
    void *currentbp = (void *)*(unsigned int *)(groupsptr+sizegroup*4);

    size_t freeblocksize;
    int isFound = 0;
    while ((!isFound) && sizegroup <NUM_OF_GROUPS){
        if (VALID(currentbp)) { 
            freeblocksize = GET_SIZE(HDRP(currentbp));
            if (freeblocksize >= asize){
                isFound =1;
                return (void *) currentbp;
            }
            else{
                if (sizegroup >= NUM_OF_GROUPS-1)
                    return NULL;    
                else if (VALID(NEXT_FREEP(currentbp)))
                {
                    currentbp =(void *) NEXT_FREEP(currentbp);
                }
                else {
 //                   printf("sizegorup %d\n", sizegroup);
                    sizegroup += 1;
                    currentbp = (void *)*(unsigned int*)(groupsptr +4* sizegroup);
                }
            }
        }
        else {
            sizegroup +=1;
            currentbp = (void *)*(unsigned int*)(groupsptr +4* sizegroup);
     }
    }
    return NULL;
}


/*
 * place - allocate the block with given size. Change header and footer information and remove it from the free list, since it is no longer free. If splitting is possible, split and add the splitted free block to the appropriate free list size group. 
 */
static void place(void *bp, size_t asize){

    size_t currentblocksize = GET_SIZE(HDRP(bp));
    
    if ((currentblocksize - asize)>= (2*DSIZE)){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        removeFree(bp, findGroup(currentblocksize));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(currentblocksize - asize, 0));
        PUT(FTRP(bp), PACK(currentblocksize - asize, 0));
        addToRoot(bp);
    }
    else {
        removeFree(bp, findGroup(asize));
        PUT(HDRP(bp), PACK(currentblocksize, 1));
        PUT(FTRP(bp), PACK(currentblocksize, 1));
        
   }

}

/*
 * mm_free - free the current block by fixing header and footer information and coalesce with adjacent blocks if possible
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
 * mm_realloc - If the requested size is smaller than the current size, then simply return the current pointer, since there needs no change. However, if the requested size is bigger, we will check if the next block is free to quickly merge two free blocks to accomodate the new size. If not free, then we simply run mm_malloc to find a new free block and mm_free the old block. Key point here is to use the fact that realloc trace files contain repeated reallocation of the same block with bigger size. I simply counted the number of times "realloc-in-place" failed (meaning that mm_malloc had to be called to find a new free block), and if the number of this failure is above some number (it is a parameter that can be adjusted), then start allocating double the size of the realloc request. This improved the throughput significantly.    
 */

void *mm_realloc(void *ptr, size_t size)
{
    //mm_check();
    void *oldptr = ptr;
    void *newptr;
    size_t currentblocksize = GET_SIZE(HDRP(ptr));

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

               removeFree(NEXT_BLKP(oldptr), findGroup(GET_SIZE(NEXT_BLKP(oldptr))));
                PUT(HDRP(oldptr), PACK(currentblocksize, 0));
                PUT(FTRP(oldptr), PACK(currentblocksize, 0));
                // coalesce with the next one
                coalesce(oldptr);
                // don't need to copy memory, just place
              place(oldptr, asize);
              return oldptr;

            }
            
        }

        // ARE YOU SERIOUS!!!!!!!!!!!!!!for realloc-bal.rep ....WHAT THE HECK THIS SOLVED EVERYTHING.......
        // aksdjfklsajflksjfklfkdj
        if (countrealloc >= 50)
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

    return NULL;
}


int mm_check(void)
{

    /*
    // If needed :  print out root free blocks of all 12 free lists
    int groupnum;
    for (groupnum=0; groupnum<NUM_OF_GROUPS; groupnum++)
    {
        printf("%p\n", (void *)*(unsigned int*)(groupsptr + 4*groupnum));
    }
    */

    int j;
    void * i;
    int freeblockcounter1 = 0;

    // iterate through all 12 free lists to check multiple things
    // count total free blocks in free lists
    // print free block address, its previous, next linked free block addresses.
    for (j =0; j<NUM_OF_GROUPS; j++) {
        for (i = (void *)*(unsigned int*)(groupsptr+ 4*j) ; VALID(i); i = NEXT_FREEP(i)) {
            freeblockcounter1 +=1;
           // printf("freeblock at %p prevfree %p nextfree %p\n", i, (void *)PREV_FREEP(i), (void *)NEXT_FREEP(i));

            // is every block in the free list marked free?
            if (GET_ALLOC(HDRP(i)) != 0) {
                printf("Free list check : Block at %p is not marked free\n", i);
            }
            // are there contiguous free blocks that has not been coalesced
            if (VALID(PREV_BLKP(i))){
                if (GET_ALLOC(HDRP(PREV_BLKP(i))) == 0) {
                    printf("Coalsece check : Not coalesced with previous block at %p\n", PREV_BLKP(i));
                }
            }
   
    // do pointers in the free list point to valid free blocks?
        // if i is not a root 
            void * prevptr = (void *) PREV_FREEP(i);
            void * nextptr = (void *)NEXT_FREEP(i);
            if (!VALID(prevptr)){
                if (i != (void *)*(unsigned int *)(groupsptr + 4*j)){ 
                    printf("Previous free block is not valid %p\n", prevptr);
                }
            } else {
                //check if prev free block is actually free
                if (GET_ALLOC(HDRP(prevptr)) != 0){
                    printf("Previous free block is not free %p\n", prevptr);
                }
            }
        // check if next free block in the list is really free
            if (VALID(nextptr)){
                if (GET_ALLOC(HDRP(nextptr)) != 0){
                    printf("Next free block is not free %p\n", nextptr);
                }
            }
       }
    }
    int freeblockcounter2 = 0;
    // is every free block actually in the free list?
    for ( i = heap_listp; VALID(i); i = NEXT_BLKP(i)){
        if (GET_ALLOC(HDRP(i)) == 0){
        //    printf("in entire list, free block found at %p\n", i);
            freeblockcounter2 += 1;
        }
    }
    // check if the number of free block in free list match the number of free block in entire list
    if (freeblockcounter1 != freeblockcounter2){
        printf("Free list vs actual free blocks: freelist %d, entire list %d\n", freeblockcounter1, freeblockcounter2);
   }
    return 1;

}

/*
 * addToRoot - add the new free list to the beginning of the appropriate size free list group
 */
static void addToRoot (void *bp)
{
    // find the right size group
    size_t sizegroup = findGroup(GET_SIZE(HDRP(bp)));
    PUT_ADDR(bp,(void *) *(unsigned int *)(groupsptr+4*sizegroup));
    PUT_ADDR(ADD_W(bp), NULL);
    if (*(unsigned int *)(groupsptr+4*sizegroup) !=(unsigned int) NULL)
        PUT_ADDR(ADD_W(*(unsigned int *)(groupsptr+4*sizegroup)), bp);
    *(unsigned int *)(groupsptr+4*sizegroup) = (unsigned int )bp; 
    //mm_check();
}

/*
 * removeFree - remove the free block from the free list group. But before leaving, it needs to make sure its previous and next free blocks are linked, if there exists previous and next free blocks. 
 */
static void removeFree (void *bp, int sizegroup)
{
    void *prevfree = (void *)PREV_FREEP(bp);
    void *nextfree = (void *)NEXT_FREEP(bp);
    
    //PUT_ADDR(bp, NULL);
    //PUT_ADDR(ADD_W(bp), NULL);
    if (!VALID(prevfree) && !VALID(nextfree)){
         *(unsigned int *)(groupsptr + 4*sizegroup) = (unsigned int)NULL;
    }
    else if (!VALID(prevfree)){
        PUT_ADDR(ADD_W(nextfree), NULL);
       *(unsigned int *)(groupsptr+4*sizegroup)= (unsigned int) nextfree;
    }
    else if (!VALID(nextfree)){
        PUT_ADDR(prevfree, NULL);
    }
    else {
        PUT_ADDR(prevfree, nextfree);
        PUT_ADDR(ADD_W(nextfree), prevfree);
        
    }


}

/*
 * findGroup - given the size, find the right size group out of 12 groups. Using binary search improved performance (although the code is kind of long...), rather than just a linear time search. 
 */

int findGroup(size_t size)
{
    
    /*
    int i;
    size_t sizegroup = -1;
    if (size > (1<<(NUM_OF_GROUPS)))
    {    sizegroup = NUM_OF_GROUPS-1; // if the size is bigger than 2^15 then it is in group 14 (last group)
        return sizegroup;
    }
    else {
        for (i = 0; i<NUM_OF_GROUPS; i++)
        {
            if (size <= (1<<(i+4))) {
                sizegroup = i;
                return sizegroup;
            }
        }
    }
    return sizegroup;
*/
    // above is too slow, linear time.....use binary search!!!!
    if (size <= 512)
    {
        if (size <=64)
        {
            if (size <=16)
                return 0;
            else {
                if (size <=32)
                    return 1;
                else 
                    return 2;
            }
        }
        else {
            if (size <= 128)
            {
                return 3;
            }
            else 
            {
               if (size <= 256) {
                  return 4;
               }
               else {
                   return 5;
               }

            }
        }
    }
    else {
        if (size <= 4096) {
            if (size <=1024) {
                return 6;
            }
            else 
            {
                if (size <=2048)
                    return 7;
                else 
                    return 8;
            }
        }
       else {
        if (size <= 8192){
            return 9;
        }
        else {
            if (size <=16384)
                return 10;
            else 
                return 11;
        }
       }
    }
}

