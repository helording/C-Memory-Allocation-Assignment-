// COMP1521 18s1 Assignment 2
// Implementation of heap management system

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myHeap.h"

// minimum total space for heap
#define MIN_HEAP  4096
// minimum amount of space for a free Chunk (excludes Header)
#define MIN_CHUNK 32

#define ALLOC     0x55555555
#define FREE      0xAAAAAAAA

typedef unsigned int uint;   // counters, bit-strings, ...

typedef void *Addr;          // addresses (&)

typedef struct {             // headers for Chunks
   uint  status;             // status (ALLOC or FREE)
   uint  size;               // #bytes, including header
} Header;

static Addr  heapMem;        // space allocated for Heap
static int   heapSize;       // number of bytes in heapMem
static Addr *freeList;       // array of pointers to free chunks
static int   freeElems;      // number of elements in freeList[]-unused
static int   nFree;          // number of free chunks

// initialise heap
int initHeap(int size)
{
	// if the size if less than the minimum, size equals minimum size
    if (size < MIN_HEAP) {
        size = MIN_HEAP;
    }
    // round up heapSize to the nearest integer divisible by 4
    if (size % 4 != 0) size = (size/4 +1 )* 4;
    
    // set heapSize global variable
    heapSize = size;
    
    //allocate memory for the heap, if there's no space return NULL
    heapMem = malloc(heapSize);
    if (heapMem == NULL) return -1;
    
    // allocate memory for the free list, if there's no space return NULL
    freeList = malloc(size/MIN_CHUNK);
    if (freeList == NULL) return -1;
    
    // set header variables and zero out freeList
    ((Header *) heapMem)->status = FREE; 
    ((Header *) heapMem)->size = heapSize;
    memset(heapMem + sizeof(Header), 0, heapSize - sizeof(Header));
    
    // make the start of memory the free space header and change global variables
    freeList[0]= heapMem;
    freeElems = 1;
    nFree = 1;
    
    return 0; // this indexust keeps the compiler quiet
}

// clean heap
void freeHeap()
{
    free(heapMem);
    free(freeList);
}

void *myMalloc(int size){

   // variables for function
   int i;
   Header *freeList_header = NULL;
   int first_potential_chunk = 0;
   int free_chunk_index = 0;
   Addr smallest_potential_chunk = NULL;  
   int block_size = size + sizeof(Header); 
   int offset = 0;
   Addr new_chunk;
   int original_block_size = 0;
   
   // if the size of the malloc less than 1 return NULL
    if (size < 1){
        return NULL;
    }
    // if chunk if not a modulos 4, round up
    if(size % 4 != 0){
        size += 4 - (size % 4);
    }
 
    uint smallestSize = 0;
    // Finds the smallest avaible free memory block to malloc
    for (i = 0; i < nFree; i++){						
   		freeList_header = (Header *)freeList[i];
   		// if memory block is greater than minimum size continue
   		if (freeList_header->size >= size + sizeof(Header)){	
   			// see if the new potential chunk is smallest	
   			if (freeList_header->size < smallestSize && first_potential_chunk != 0){	
   				smallest_potential_chunk = freeList[i];
   				smallestSize = freeList_header->size;
   				free_chunk_index = i;
   			}
   			// if the block is the first potential chunk 
   			if (first_potential_chunk == 0){					
   				smallest_potential_chunk = freeList[i];
   				first_potential_chunk++;
   				free_chunk_index = i;						
   			}
   		}	   		
   }
   // if there is no possible free chunk, return NULL
   if (smallest_potential_chunk == NULL){
   		return NULL;
   }
    
    // get the header
    Header *alloc_header = (Header *)smallest_potential_chunk;
    if (alloc_header->size <= size + sizeof(Header) + MIN_CHUNK){
        // change the status to ALLOC
        alloc_header->status = ALLOC;
        // move the freeList elements down from where index of the new allocated block
        for (i = 0; i < nFree - 1; i++){							
			if (i == free_chunk_index){
				while (i < nFree - 1){
					freeList[i] = freeList[i + 1];
					i++;
				}
				break;
			}
		}
		freeList[nFree - 1] = NULL;
        // decrease number of free chunks by 1
        nFree--;
    } else {
		// change header values 
   		alloc_header->status = ALLOC; 						
   		original_block_size = alloc_header->size;				
   		alloc_header->size = block_size;	
   		
   		// finding location of new free chunk and setting it's values
   		offset = heapOffset(smallest_potential_chunk);		
   		offset = offset + block_size;		
   		new_chunk = heapMem + offset;						
   		((Header *)(new_chunk))->status = FREE;				
   		((Header *)(new_chunk))->size = original_block_size - size - sizeof(Header);  
		freeList[free_chunk_index] = new_chunk;
    }
    return smallest_potential_chunk + sizeof(Header);
}

void myFree(void *block)
{
   // if block equals NULL exit
   if(block == NULL){
        fprintf(stderr, "Attempt to free unallocated chunk\n");
        exit(1);
   }
   
   // indexs for loops
   int i = 0;
   int next_block_index;
   
   // casting the block pointer to a Header variable 
   Addr block_header = (void *)block - sizeof(Header);
   Header *alloc_header = (Header *)(block_header);
   
   // if block not pointing to a Header, exit
   if (alloc_header->status != ALLOC){
   		fprintf(stderr, "Attempt to free unallocated chuck\n");
   		exit(1);
   	}
   
   // initialization of header variables for loop and if statments 
   Header *loop_header = (Header *)heapMem;
   Header *previous_header = NULL;
   previous_header = loop_header;
   
   // locating the point of the block to find the previous block header
   while (loop_header != alloc_header){
   			previous_header = loop_header;
   			i = loop_header->size; 
   			loop_header = (void *)loop_header + i;
   			
   }

   // finding the next block's header
   Header *next_header = (void *)alloc_header + alloc_header->size;
   
   // if next block is beyond Memory space, next header equal NULL
   if (heapOffset(next_header) == -1){     
   		next_header = NULL;
   } 
   
   // if the next block is free, find it's index on the freeList
   if (next_header->status == FREE) {
   		for (next_block_index = 0; next_header != freeList[next_block_index]; next_block_index++){
   		}
   }

   // if statements for the allocated blocks surroundings to determine how to rearrange memory space
   
   //  if the surrounding blocks are allocated, or if the block is at the start or at the end of the memory space and the block next to it is allocated 
   if ( (previous_header == NULL && next_header->status == ALLOC) || (next_header == NULL && previous_header->status == ALLOC) || (previous_header->status == ALLOC && next_header->status == ALLOC)) {
   		// find where the next free block is after block
   		int free_list_index = 0;
        for(free_list_index = 0; free_list_index < nFree; free_list_index++){
            if((void *)alloc_header < freeList[free_list_index]){ 
                break;
            }
        }
        // shift the freeList elements down from the point of the former free block pointer
        for (int i = nFree; i > free_list_index; i--){
            freeList[i] = freeList[i - 1];
        }
        freeList[free_list_index] = alloc_header;
   		nFree++;
		alloc_header->status = FREE;
   		memset((void *)alloc_header + sizeof(Header), 0, alloc_header->size - sizeof(Header));
   
   // if the block is surrounded by two free blocks
   } else if (previous_header->status == FREE && next_header->status == FREE) {
   		// remove the allocated block's pointer from the freeList   			
   		for (i = 0; i < nFree - 1; i++){								
			if (i == next_block_index){
				while (i < nFree - 1){
					freeList[i] = freeList[i + 1];
					i++;
				}
				break;
			}
		}
		freeList[nFree - 1] = NULL; 
		
		nFree--;
		previous_header->size += alloc_header->size + next_header->size;
		previous_header->status = FREE;
   		memset((void *)previous_header + sizeof(Header), 0, previous_header->size - sizeof(Header));
   
   // if the previous block is allocated and the next block is free, or if the block is the first block in the heap and the next block is free
   } else if ( (previous_header == NULL && next_header->status == FREE) || (previous_header->status == ALLOC && next_header->status == FREE) ) {
   		// move pointer in freeList from pointing at the next block, to pointing at freed block 
   		for (i = 0; i < nFree; i++){
   			if (freeList[i] == next_header){
   				freeList[i] = alloc_header;
   				break;
   			}
   		}
   		alloc_header->status = FREE;
   		alloc_header->size += next_header->size; 
   		memset((void *)alloc_header + sizeof(Header), 0, alloc_header->size - sizeof(Header));
   	
   	// if the previous block is free and the next block is allocated, or if the block is the last block in the heap and the previous block is free
   	} else if ( (next_header == NULL && previous_header->status == FREE) || (previous_header->status == FREE && next_header->status == ALLOC) ) {
   		previous_header->size += alloc_header->size;
   		memset((void *)previous_header + sizeof(Header), 0, previous_header->size - sizeof(Header));
   		
   	}
}
// convert pointer to offset in heapMem
int  heapOffset(void *p)
{
   Addr heapTop = (Addr)((char *)heapMem + heapSize);
   if (p == NULL || p < heapMem || p >= heapTop)
      return -1;
   else
      return p - heapMem;
}

// dump contents of heap (for testing/debugging)
void dumpHeap()
{
   Addr    curr;
   Header *chunk;
   Addr    endHeap = (Addr)((char *)heapMem + heapSize);
   int     onRow = 0;

   curr = heapMem;
   while (curr < endHeap) {
      char stat;
      chunk = (Header *)curr;
      switch (chunk->status) {
      case FREE:  stat = 'F'; break;
      case ALLOC: stat = 'A'; break;
      default:    fprintf(stderr,"Corrupted heap %08x\n",chunk->status); exit(1); break;
      }
      printf("+%05d (%c,%5d) ", heapOffset(curr), stat, chunk->size);
      onRow++;
      if (onRow%5 == 0) printf("\n");
      curr = (Addr)((char *)curr + chunk->size);
   }
   if (onRow > 0) printf("\n");
}
