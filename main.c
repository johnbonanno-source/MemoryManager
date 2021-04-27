/**
john bonanno
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// number of characters to read for each line from input file
#define BUFFER_SIZE         10

// number of bytes to read
#define CHUNK               256

FILE *address_file;
FILE *backing_store;

// how we store reads from input file
char address [ BUFFER_SIZE ] ;
// the logical address
int logical_address ;
// we store page num here to preserve the
// logical address for printouts/error
int page_number ;
// we will store the offset here
int offset ;
// the buffer containing reads from backing store
signed char buffer[ CHUNK ] ;
// the value of the byte (signed char) in memory
signed char value;
// 16 1's in HEX
const int firstmask = 0xFFFF;
// 8 1's in hex
const int secondmask = 0xFF ;
// we will shift by 8 after
// storing the page number
// to get the offset
const int shift = 8 ;

int tlbHit = 0;
int tlbMiss = 0;
int iterations = 0;

int pageFrameCount = 16;
const int tlbSize = 16;
unsigned int tlbFrame[tlbSize]={-1};
unsigned int tlbPage[tlbSize];
int elementCount = 0;

struct tlb {
    int tlbFrame;
    int tlbPage;
};


//a 2d array representing our physical memory
signed char physicalMemory [ CHUNK ][ CHUNK ] ;
//an array representing our page table.
const int pageTableSize = 256 ;
signed int pageTable[ pageTableSize ] ;


int frameNumber = 0 ;
int pageFaults = 0 ;
int i = 0;
//num elements in tlbframe
int elementCount;
int frameVal = 0;
//read chunk from backing store
//store it in page frame
//each frame is 256 bytes
//get frame # with counter
//2d array for the physical memory

//offset = where in that byte you want to get the data from
//page 15 goes to frame 0
//If YOU reference 15 again, you will see in the page table page 15 is mapped to0
// so you dont have to go to the backing store.


int main( int argc , char *argv[] ){

    struct tlb tlbTable[tlbSize];

    for (int i = 0; i < tlbSize ; i++) {

        tlbTable[ i ].tlbPage = -1;
        tlbTable[ i ].tlbFrame = -1;
    }

    //set the uninitialized frames to be equal to -1
    //for some reason the one - liner to init - pageTable[256]={-1}; was not working...
    for( int i = 0 ; i < pageTableSize ; i ++ ){
        pageTable[ i ] = -1 ;
    }

    // perform basic error checking
    if ( argc != 3 ) {

        fprintf( stderr , "Usage: ./vm [ backing store ] [ input file ]\n" ) ;

        return -1 ;
    }
    // open the file containing the backing store
    backing_store = fopen ( argv[ 1 ] , "rb" ) ;

    if ( backing_store == NULL ) {

        fprintf( stderr, "Error opening %s\n" , argv[ 1 ] );

        return -1;
    }

    // open the file containing the logical addresses
    address_file = fopen ( argv[ 2 ], "r" ) ;

    // if we can't open the address_file give error code
    if ( address_file == NULL ) {

        fprintf( stderr, "Error opening %s\n" , argv[ 2 ] ) ;

        return -1 ;
    }



    // read through the input file and output each logical address
    // and retrieve the corresponding page frame of each page_number
    while ( fgets ( address, BUFFER_SIZE, address_file ) != NULL ) {

        iterations++;

        logical_address = atoi( address ) ;
        // ignore everything but 16 low-order bits
        logical_address = logical_address & firstmask ;
        //printf( "\n Virtual Address: = %d ", logical_address ) ;

        // shift by 8 to get the page number
        page_number = ( logical_address >> shift ) ;
        //printf( "\n page number = %d\n ", page_number ) ;
        // to get the offset ( low order 8 bits ) :
        offset = logical_address & secondmask ;
        //printf( "offset = %d\n ", offset ) ;

        // first seek to byte CHUNK in
        // the backing store
        // SEEK_SET in fseek() seeks from the beginning of the file
        // we seek to adjust pointer
        //if ( page fault ) : read 256 bytes from the page frame
        if ( fseek( backing_store, CHUNK*page_number, SEEK_SET ) != 0 ) {

            fprintf( stderr, "Error seeking in backing store\n" ) ;

            return -1 ;
        }
        // now read CHUNK (page size) bytes from the backing store to the buffer
        if ( fread( &physicalMemory[ frameNumber ] , sizeof( signed char ) , CHUNK , backing_store ) == 0 ) {

            fprintf( stderr, "Error reading from backing store\n" ) ;

            return -1;
        }


        //check the tlb
        int ind = -1;
        for ( int i = 0; i < tlbSize ; i++ ) {
            if ( tlbTable[ i ].tlbPage == page_number ) {
                ind = tlbTable[ i ].tlbFrame;
                tlbHit++;
            }
        }
        //if theres tlb miss
        if( ind==-1 ){
            tlbTable[ elementCount ].tlbPage = page_number;
            tlbTable[ elementCount ].tlbFrame = frameVal;
            tlbMiss++;
        }

        elementCount = elementCount % 15;

        // now to check if the page table has our
        // page number in it already if there was tlb+page miss

        if ( pageTable[ page_number ] == -1 ) {

            //increase the count of page faults
            pageFaults++;
            frameVal++;
            //we must update page table because there was a page fault.
            pageTable[ page_number ] = frameNumber ;
            frameNumber++ ;
            elementCount++;

        }

        //now we have corresponding page frame stored @ page table[page_number]
        // the value of the page frame can be accessed by getting the frame number
        // from the pageTable by searching for corresponding page frame
        //i = ptable[pagenumber]
        i = pageTable[ page_number ] ;
        //and by using that value to index our physical memory
        int pageFrameVal = physicalMemory[ i ][ offset ] ;


        printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, i*CHUNK+offset, pageFrameVal);

    }
    printf("Number of translated addresses: %d\n ", iterations);
    printf( "page  faults : %d\n pg fault rate: %f\n tlb hit: %d tlb hit rate %f", pageFaults, ((pageFaults*1.0)/iterations), tlbHit , ((tlbHit*1.0)/iterations )) ;
    //for fread() physical mem from backing backingstore current =  &physical_memory[frame]]

    fclose ( address_file ) ;
    fclose ( backing_store ) ;

    return 0 ;
}

