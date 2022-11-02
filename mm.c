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
    "team 6",
    /* First member's full name */
    "Narin Jung",
    /* First member's email address */
    "nalynnshinwhwa@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constant and macros*/
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x):(y))

/* Pack and write a word at address p */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp(usable block), compute address of its predecessor and successor*/
#define PREP(bp) (*(void**)(bp))
#define SUCP(bp) (*(void**)(bp+WSIZE))

#define SEGP(ptr, list) (*(void**)(ptr+(list*WSIZE)))
#define SEG_SUCP(bp) (*(void**)(bp))

#define LISTLIMIT 20

/* Global variables */
static void* heap_listp;
// static void* free_listp;
static void* segre_listp;
// static void* segregation_list[LISTLIMIT];

/* Function prototype */
int mm_init(void);
static void *extend_heap(size_t extend_size, size_t asize, char list);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
void put_free_block(void *bp, size_t size);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    if ((heap_listp = mem_sbrk(22 * WSIZE)) == (void *)-1)
        return -1;
    // PUT(heap_listp, 0);/*anonymous padding*/
    PUT(heap_listp, PACK(22*WSIZE, 1));/*prologue header*/
    segre_listp = heap_listp + (1 * WSIZE);
    int list = 0;
    for (list; list < 12; list++)
    {
        // printf("LIST: %d\n", list);
        SEGP(segre_listp, list) = extend_heap(CHUNKSIZE, 1 << list, list);
    }
    // SEGP(segre_listp, list) = NULL;
    PUT(heap_listp + (22 * WSIZE), PACK(22 * WSIZE, 1)); /*prologue footer*/
    heap_listp += (22 * WSIZE);
    return 0;
}

static void *extend_heap(size_t extend_size, size_t asize, char list){
    char *bp ;
    if ((long)(bp = mem_sbrk(extend_size)) == -1)
        return NULL;

    PUT(bp, PACK(asize, 0));
    char *tmp_bp;
    if (asize > (CHUNKSIZE >> 1))
        tmp_bp = bp + CHUNKSIZE;
    else
        tmp_bp = bp + asize;

    char *result_bp = tmp_bp;
    SEGP(segre_listp, list) = result_bp;

    for (tmp_bp; tmp_bp < extend_size + bp - asize; tmp_bp += asize)
    {
        SEG_SUCP(tmp_bp) = tmp_bp + asize;
    }
    SEG_SUCP(tmp_bp) = NULL;

    return result_bp;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    printf("%d\n", size);
    size_t asize = 1;
    size_t extend_size;
    char *bp;
    char list = 0;

    if (size == 0) return NULL;
    if (size <= DSIZE)
        asize = DSIZE;
    else{
        while (list < LISTLIMIT && size > 1){
            list++;
            size >>= 1;
        }
        list += 1;
        asize <<= list;      
    }

    if ((list < LISTLIMIT- 1) && SEGP(segre_listp, list) != NULL){
        
        bp = SEGP(segre_listp, list);
        SEGP(segre_listp, list) = SEG_SUCP(bp);
        
        return bp;
    }

    if (asize > (CHUNKSIZE>>1)){
        extend_size = CHUNKSIZE + ((asize + (CHUNKSIZE - 1)) / CHUNKSIZE) * CHUNKSIZE;
    }else{
        extend_size = CHUNKSIZE;
    }

    if((bp = extend_heap(extend_size, asize, list)) == NULL)
        return NULL;
    SEGP(segre_listp, list) = SEG_SUCP(bp);
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    printf("free\n");

    // printf("%d\n", ptr - heap_listp);
    // printf("%d\n", ((ptr - heap_listp) / (CHUNKSIZE)));
    // printf("%d\n", ((ptr - heap_listp) / (CHUNKSIZE)) * (CHUNKSIZE));
    // printf("%d\n", GET_SIZE(heap_listp + ((ptr - heap_listp) / (CHUNKSIZE)) * (CHUNKSIZE)));

    size_t blocksize;
    if ((GET(ptr) % CHUNKSIZE) == 0){

        // printf("%d\n", ptr - heap_listp);
        // printf("%d\n", ((ptr - heap_listp) / (CHUNKSIZE)));
        // printf("%d\n", ((ptr - heap_listp) / (CHUNKSIZE)) * (CHUNKSIZE));
        // printf("%d\n", GET_SIZE(heap_listp + ((ptr - heap_listp) / (CHUNKSIZE)) * (CHUNKSIZE)) - CHUNKSIZE);

        blocksize = GET_SIZE(heap_listp + (((ptr - heap_listp) / (CHUNKSIZE)) * (CHUNKSIZE)) - CHUNKSIZE);
    
    }else{
        blocksize = GET_SIZE(heap_listp + ((ptr - heap_listp) / (CHUNKSIZE)) * (CHUNKSIZE));
    }
    put_free_block(ptr, blocksize);
}

void put_free_block(void *bp, size_t size){

    int list = 0;
    while(list < LISTLIMIT && size > 1){
        list++;
        size >>= 1;
    }

    SEG_SUCP(bp) = SEGP(segre_listp, list);
    SEGP(segre_listp, list) = bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
//     void *oldptr = ptr;
//     void *newptr;
//     size_t copySize;
    
//     newptr = mm_malloc(size);
//     if (newptr == NULL)
//       return NULL;
//     //copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
//     copySize = GET_SIZE(HDRP(oldptr));
//     if (size < copySize)
//         copySize = size;
//     memcpy(newptr, oldptr, copySize);
//     mm_free(oldptr);
//     return newptr;
}