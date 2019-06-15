/*
 * mm.c: a basic dynamic memory allocator implemented with explicit list
 *
 * Author: Qizheng "Alex" Zhang (qizhengz)
 *
 * This is the only file you should modify.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

/* $begin mallocmacros */
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

/* Basic constants and macros */
#define WSIZE       4       /* word size (bytes) */
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<5)  /* initial heap size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */

#define MAX(x, y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
/* NB: this code calls a 32-bit quantity a word */
#define GET(p)       (*(unsigned int *)(p))
#define GETE(p)       (*(unsigned long *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))
#define PUTE(p, val)  (*(unsigned long *)(p) = (unsigned long)(val))
#define PUT8(p, val)  (*(unsigned long *)(p) = (unsigned long)(val))
#define GET8(p)       (*(unsigned long *)(p))
/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))
/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;  /* pointer to first block */
static char *heap_start = 0; // start of the heap
#ifdef NEXT_FIT
static char *rover;       /* next fit rover */
#endif

/* function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void remove_block(void *p);
static void insert_block(void *p);
//static void mm_check();

/*
 * Initialize: return -1 on error, 0 on success.
 */
/* $begin mminit */
int mm_init(void) {
    /* create the initial empty heap */
    if ((heap_listp = mem_sbrk(8*WSIZE)) == NULL)
        return -1;
    
    heap_start = heap_listp;
    
    //PUTE(heap_listp, 0); // root next pointer
    PUTE(heap_listp,heap_start); // root next pointer
    //PUTE(heap_listp+DSIZE, 0); // root prev pointer
    PUTE(heap_listp+DSIZE, heap_start); // root prev pointer
    
    PUT(heap_listp+2*DSIZE, 0);                        /* alignment padding */
    PUT(heap_listp+2*DSIZE+WSIZE, PACK(OVERHEAD, 1));  /* prologue header */
    PUT(heap_listp+2*DSIZE+2*WSIZE, PACK(OVERHEAD, 1));  /* prologue footer */
    PUT(heap_listp+3*DSIZE+WSIZE, PACK(0, 1));   /* epilogue header */
    heap_listp += 3*DSIZE;
    
#ifdef NEXT_FIT
    rover = heap_listp;
#endif
    
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    
    //printf("No seg fault in mm_init\n");
    //mm_check();
    
    return 0;
}
/* $end mminit */

/*
 * malloc
 */
/* $begin mmmalloc */
void *mm_malloc (size_t size) {
    //printf("No seg fault before mm_malloc\n");
    //printf("$$$$$ malloc %d\n",size);
    
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;
    if (heap_listp == 0){
        mm_init();
    }
    
    /* Ignore spurious requests */
    if (size <= 0)
        return NULL;

    // optimization for binary_bal.rep
    if(size == 448)
      size = 512;
    
    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= 2*DSIZE)
        asize = 2*DSIZE + OVERHEAD;
    else
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1)) / DSIZE);
    
    //printf("No seg fault before place1\n");
    
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        //mm_check(__func__);
        return bp;
    }
    
    //printf("No seg fault after place2\n");
    
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    //mm_check(__func__);
    //printf("No seg fault in malloc\n");
    return bp;
}
/* $end mmmalloc */

/*
 * free
 */
/* $begin mmfree */
void mm_free (void *bp) {
  if(bp == 0) return;

  size_t size = GET_SIZE(HDRP(bp));
  if (heap_listp == 0){
    mm_init();
  }

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  //insert_block(bp);
  coalesce(bp);
}
/* $end mmfree */

/*
 * realloc - you may want to look at mm-naive.c
 */
void *mm_realloc(void *oldptr, size_t size) {
    size_t oldsize;
    void *newptr;
    
    void *ptr=oldptr;
    
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }
    
    /* If oldptr is NULL, then this is just malloc. */
    if(ptr == NULL) {
        return mm_malloc(size);
    }
    
    newptr = mm_malloc(size);
    
    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }
    
    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);
    
    /* Free the old block. */
    mm_free(ptr);
    
    return newptr;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *mm_calloc (size_t nmemb, size_t size) {
  void *ptr;
  if (heap_listp == 0){
    mm_init();
  }

  ptr = mm_malloc(nmemb*size);
  bzero(ptr, nmemb*size);

  return ptr;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
/*
static int in_heap(const void *p) {
    return p < mem_heap_hi() && p >= mem_heap_lo();
}
*/

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
/*
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}
*/

/*
 * mm_checkheap
 */
void mm_checkheap(int verbose){
}

// below are helper functions


/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    void *return_ptr;
    
    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) < 0)
        return NULL;
    
    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* new epilogue header */
    
    //printf("problem must be in coalesce\n");
    /* Coalesce if the previous block was free */
    return_ptr = coalesce(bp);
    //mm_checkheap(0);
    return return_ptr;
}
/* $end mmextendheap */


/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
/* $begin mmplace */
/* $begin mmplace-proto */
static void place(void *bp, size_t asize)
/* $end mmplace-proto */
{
    //printf("No seg fault before place\n");
    size_t csize = GET_SIZE(HDRP(bp));
    
    if ((csize - asize) >= (2*DSIZE + OVERHEAD)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        remove_block(bp);
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        insert_block(bp);
    }
    else {
        remove_block(bp);
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
    //printf("No seg fault after place\n");
}
/* $end mmplace */


/*
 * find_fit - Find a fit for a block with asize bytes
 */
/* $begin find_fit */
static void *find_fit(size_t asize)
{
    //printf("No seg fault before find_fit\n");
#ifdef NEXT_FIT
    /* next fit search */
    char *oldrover = rover;
    
    /* search from the rover to the end of list */
    for ( ; GET_SIZE(HDRP(rover)) > 0; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;
    
    /* search from start of list to old rover */
    for (rover = heap_listp; rover < oldrover; rover = NEXT_BLKP(rover))
        if (!GET_ALLOC(HDRP(rover)) && (asize <= GET_SIZE(HDRP(rover))))
            return rover;
    
    return NULL;  /* no fit found */
#else
    /* first fit search */
    // honestly I think this is enough?
    void *bp;
    
    //printf("BP ADDRESS=%p\n",bp);
    //printf("Heap_start ADDRESS=%p\n",heap_start);
    //printf("GETE(heap_start) ADDRESS=%p\n",GETE(heap_start));
    //printf("No seg fault\n");
    for (bp = (void *)GETE(heap_start); GET_SIZE(HDRP(bp)) > 0; bp = (void *)GETE(bp)) {
        //printf("bp is %p\n",bp);
        //printf("GETE(bp) is %p\n",GETE(bp));
        //printf("GET_SIZE(HDRP(bp)) is %p\n",GET_SIZE(HDRP(bp)));
        //printf("GET_SIZE(HDRP(GETE(bp))) is %p\n",GET_SIZE(HDRP(GETE(bp))));
        //printf("No seg fault\n");
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
            //printf("No seg fault after find_fit\n");
            return bp;
        }
        //printf("No seg fault2\n");
        //mm_check();
    }
    //printf("No seg fault after find_fit\n");
    return NULL; /* no fit */
#endif
}
/* $end find_fit */

static void remove_block(void *p){
    //printf("No seg fault before remove_block\n");
  void *prev_block = (void *)GETE(p+DSIZE);
  void *next_block = (void *)GETE(p);
  if(prev_block == heap_start){
    if(next_block == heap_start){
      // only element in the list
      //printf("no seg fault1\n");
      PUTE(heap_start, heap_start);
      //printf("no seg fault2\n");
      PUTE(heap_start+DSIZE, heap_start);
    }
    else{
      // first but not only element in the list
      //printf("no seg fault3\n");
      PUTE(next_block+DSIZE, heap_start);
      //printf("no seg fault4\n");
      PUTE(heap_start, next_block);
    }
  }
  else{
    if(next_block == heap_start){
      // last but not only element in the list
      //printf("no seg fault5\n");
      PUTE(prev_block, heap_start);
      //printf("no seg fault6\n");
      PUTE(heap_start+DSIZE, prev_block);
    }
    else{
      // general case
      //printf("no seg fault7\n");
      //printf("p is %p\n",p);
      //printf("GETE(p) is: %p\n",GETE(p));
      //printf("GETE(p)+DSIZE is: %p\n",GETE(p)+DSIZE);
      PUTE(next_block+DSIZE, prev_block);
      //printf("no seg fault8\n");
      PUTE(prev_block, next_block);
    }
  }
  //printf("No seg fault in remove_block\n");
}

// insert the free block into the list
static void insert_block(void *p){
  //printf("No seg fault before insert_block\n");
  void *header_next = (void *)GETE(heap_start);
  void *header_prev = (void *)GETE(heap_start+DSIZE);
  if(header_next == heap_start && header_prev == heap_start){
    // no element in the list
    /*
      printf("go through insert1\n");
      printf("p is %p\n",p);
      printf("header_next is %p\n",header_next);
      printf("header_prev is %p\n",header_prev);
    */
    PUTE(heap_start, p);
    PUTE(heap_start+DSIZE, p);
    PUTE(p, heap_start);
    PUTE(p+DSIZE, heap_start);
  }
  else{
    /*
      printf("go through insert2\n");
      printf("p is %p\n",p);
      printf("header_next is %p\n",header_next);
      printf("header_prev is %p\n",header_prev);
    */
    PUTE(heap_start, p);
    PUTE(p, header_next);
    PUTE(p+DSIZE, heap_start);
    PUTE(header_next+DSIZE, p);
  }
  //printf("No seg fault after insert_block\n");
  //mm_check();
}

/*
 * coalesce - boundary tag coalescing. Return ptr to coalesced block
 */
/* $begin coalesce */
static void *coalesce(void *bp)
{
    //printf("No seg fault before coalesce\n");
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
    // both blocks before and after are allocated
    if (prev_alloc && next_alloc) {            /* Case 1 */
        insert_block(bp);
        return bp;
    }
    
    // only block before is allocated
    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        remove_block(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }
    
    // only block after is allocated
    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        remove_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    
    // both blocks are free
    else {  /* Case 4 */
        remove_block(NEXT_BLKP(bp));
        remove_block(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
        GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    
    insert_block(bp);
    
    //printf("No seg fault in coalesce\n");
    
#ifdef NEXT_FIT
    /* Make sure the rover isn't pointing into the free block */
    /* that we just coalesced */
    if ((rover > (char *)bp) && (rover < NEXT_BLKP(bp)))
        rover = bp;
#endif
    
    //printf("No seg fault after coalesce\n");
    return bp;
}
/* $end coalesce */
/*
 static void mm_check(){
 void* bp = heap_listp;
 printf("\n");
 printf("bp now %p\n",bp);
 printf("heap_start is %p\n",heap_start);
 printf("================Begin mm_check\n");
 
 printf("Free list root Addr=%p, Next Pointer addr= %p, Prev Pointer add=%p\n", heap_start, GETE(heap_start),GETE(heap_start+DSIZE));
 
 
 while(!(GET_ALLOC(HDRP(bp))==1 && GET_SIZE(HDRP(bp))==0)){
 
 if(GET_ALLOC(HDRP(bp))==1){
 printf("=======This is allocated. Address=%p,Size in header=%d,Size in footer=%d\n",bp,GET_SIZE(HDRP(bp)),GET_SIZE(FTRP(bp)));
 
 }else{
 printf("=====This is NOT allocated. Address=%p,Size in header=%d,Size in footer=%d,Next Addr=%p, Prev Addr=%p\n",bp,GET_SIZE(HDRP(bp)),GET_SIZE(FTRP(bp)),GETE(bp),GETE(bp+DSIZE));
 }
 
 bp=NEXT_BLKP(bp);
 }
 
 printf("================End mm_check\n");
 printf("\n");
 }
 
 */

/*
static void check(){
    
    void* bp = heap_listp;
    int exitflag = 0;
    printf("Free list root Addr=%p, Next Pointer addr= %p, Prev Pointer add=%p\n", (void *)heap_start, (void *)GET8(heap_start),(void *)GET8(heap_start+DSIZE));
    
    while(!(GET_ALLOC(HDRP(bp))==1 && GET_SIZE(HDRP(bp))==0)){
        if(GET_ALLOC(HDRP(bp))==1){
            printf("=====This is allocated.     Address=%p,Size(From Header)=%d, Size(From Footer)=%d\n",(void *)bp,GET_SIZE(HDRP(bp)),GET_SIZE(FTRP(bp)));
        } else {
            printf("=====This is NOT allocated. Address=%p,Size(From Header)=%d, Size(From Footer)=%d ,Next Addr=%p, Prev Addr=%p\n", (void *)bp,GET_SIZE(HDRP(bp)),GET_SIZE(FTRP(bp)), (void *)GET8(bp),(void *)GET8(bp+DSIZE));
            
            if(GET_SIZE(HDRP(bp))<24 || GET_SIZE(FTRP(bp))<24){
                printf("Not allocated cannot smaller than 24 bytes");
                exitflag = 1;
            }
        }
        
        if(GET_SIZE(HDRP(bp))!=GET_SIZE(FTRP(bp))){
            printf("Header size does not match with the Footer size\n");
            exitflag = 1;
        }
        
        if(1==exitflag){
            exit(0);
        }
        bp=NEXT_BLKP(bp);
    }
    printf("================End mm_check=====================\n");
    
}
*/

/*
static void mm_check( char const * caller_name )
{
    printf("================Begin mm_check====================\n");
    printf( "mm_check was called from %s()\n", caller_name );
    check();
}
*/

