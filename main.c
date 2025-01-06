#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// All these constats are extracted from the problem statement detail 
#define FRAME_SIZE 256

// Frame Size is 256 bytes 2^8
#define FRAMES 256          // Number of Frames
#define PAGE_SIZE 256       // Page Size is 2^8
#define TLB_SIZE 16         // Number of TLB entries
#define PAGE_MASKER 0xFFFF
#define OFFSET_MASKER 0xFF
#define ADDRESS_SIZE 10     // Integer address size from the address file
#define CHUNK 256           

FILE *address_file;         
FILE *backing_store;


// Struct to store TLB and page table 
// Each TLB Entry : 8 bytes 
struct page_frame{
    int page_number;
    int frame_number;
};


// Data Structure
// Physical Memory (2D Array) -> [256 x 256]

// [4 bytes][4 bytes][4 bytes][4 bytes][4 bytes] ... x 256
// [4 bytes][4 bytes][4 bytes][4 bytes][4 bytes]
// [4 bytes][4 bytes][4 bytes][4 bytes][4 bytes]
// [4 bytes][4 bytes][4 bytes][4 bytes][4 bytes]
// [4 bytes][4 bytes][4 bytes][4 bytes][4 bytes]
// .... 
// x 256

// Page Table : [256]
// [ Page Number +  Frame Number ]
// [ Page Number +  Frame Number ]
// [ Page Number +  Frame Number ]
// [ Page Number +  Frame Number ]
// ...
// x 256

// TLB : [16]
// [ Page Number +  Frame Number ]
// [ Page Number +  Frame Number ]
// [ Page Number +  Frame Number ]
// [ Page Number +  Frame Number ]
// ...
// x 16

int Physical_Memory [FRAMES][FRAME_SIZE];
// Page Table : LOGICAL address -> Physical Address
//              (Page Number)      (Frame Number)
// The size of the page table is the number of available frames in the system
struct page_frame PAGE_TABLE[FRAMES];
// TLB : Cache for Page Table
struct page_frame TLB[TLB_SIZE];


// System Global Variables
int translation_address = 0;
// Use to read address from the address file
char address[ADDRESS_SIZE];


int TLBHits = 0;
int page_fault = 0;


signed char buffer[CHUNK];
signed char value;


int firstAvailableFrame = 0;
int firstAvailablePageTableIndex = 0;

int TLB_Full_Entries = 0;


void get_page(int logical_address);
int read_from_store(int pageNummber);
void insert_into_TLB(int pageNumber, int frameNumber);


// Physical memory is 256 x 256 
// Page Table Size : 256
// Therefore logical address is 2^16 bits
// 8 bits as page table entry, 8 bits as memory offset 

void get_page(int logical_address){
    int pageNumber = ((logical_address & PAGE_MASKER) >> 8);
    int offset = (logical_address & OFFSET_MASKER);

    // First Try to get page from TLB
    int frameNumber = -1;
    int i; 
    // Look Through TLB for a match
    for (int i =0; i < TLB_SIZE; i++){
        if (TLB[i].page_number == pageNumber){  // If the TLB key is equal to the page number,
            frameNumber = TLB[i].frame_number;  // the frame number value is extracted 
            TLBHits++;                          // and TLBHit counter incremented
            printf("%d %d %d %d\n", TLBHits, pageNumber, frameNumber, logical_address);
        }
    }

    // if the frameNumber was not in the TLB (TLB miss)
    if (frameNumber == -1){
        int i ;
        for (i = 0; i < firstAvailablePageTableIndex; i++){ // Walk through the page table
            if (PAGE_TABLE[i].page_number == pageNumber){   // The size of page tavle is the same number as the frame number
               frameNumber = PAGE_TABLE[i].frame_number;    // If the page is found in those content 
            }                                               // extract the frame number from the page table
        }
        if (frameNumber == -1){
            frameNumber = read_from_store(pageNumber);      // If the page is not found in the page table
            page_fault++;                                   // Page Fault: call read_from_store to 1. get frame 
        }                                                   // from the Physical memory and the Page Table
                                                            // and 2. set the frame number to the current 
                                                            // firstAvailableFrame index 
    }
    insert_into_TLB(pageNumber, frameNumber);        // Call function to insert page number into the TLB
    value = Physical_Memory[frameNumber][offset];    // Frame number and offset used to get the signed value stored at that address

    // Testing before writing to the file 
    // printf("Page Number: %d\n", pageNumber);
    // printf("Frame Number: %d\n", frameNumber);
    // printf("Offset: %d\n", offset );
    // // Output the virtual address, physical address and value of the signed char 
    // printf("Virtual Address: %d, Physical Address: %d, Value: %d\n", 
    // logical_address, (frameNumber << 8)| offset, value);
    // TODO: Output result to the file 


}

// Assume store is a file in the disk
// Return the frame number
int read_from_store(int pageNumber){
    // First seek to byte CHUNK in the backing store 
    if (fseek(backing_store, pageNumber* CHUNK, SEEK_SET) != 0)
        fprintf(stderr, "Error reading from backing store");
    // Now read CHUNK bytes from the backing store to the buffer 
    if (fread(buffer, sizeof(signed char), CHUNK, backing_store) == 0)
        fprintf(stderr, "Errror reading from backing store");
    
    // Load the bits into the first available frame in the physical 2D Array
    int i;
    for (i = 0; i < CHUNK; i++){
        Physical_Memory[firstAvailableFrame][i] = buffer[i];
    }
    // And then the frame number into the page table in the first available frame
    PAGE_TABLE[firstAvailablePageTableIndex].page_number = pageNumber;
    PAGE_TABLE[firstAvailablePageTableIndex].frame_number = firstAvailableFrame;

    firstAvailableFrame++;
    firstAvailablePageTableIndex++;
    return PAGE_TABLE[firstAvailablePageTableIndex-1].frame_number;
}


void insert_into_TLB(int pageNumber, int frameNumber){
    // The algorithum to insert into TLB using FIFO replacement. It goes as follow :
    // First check if the page is already in the TLB table. 
    //   - If it is in the TLB -> TLB Hit
    //     Then move it to the end of the TLB. "Following the TLB Policy."
    // 
    //   - If It is not in the TLB -> TLB miss
    //     1. If TLB is not full: Add it to the empty entry
    //     2. Else Move everything one step to the left to free up the last cell and put the page into this last cell.  
    //     (Can be optimised with C++ vector)

    int i;
    // 1. Check Page Number in the TLB: 
    for (i = 0; i < TLB_Full_Entries; i++){
        if (TLB[i].page_number == pageNumber){
            // Move all entries to the left
            for (i = i; i < TLB_Full_Entries-1; i++)
                TLB[i] = TLB[i+1];
            break;
        }
    }

    // TLB Miss 
    // - If Full -> Remove the least used entry
    // - If Not Full -> Move Everything to the left and add entry to the end
    if (i == TLB_Full_Entries){
        int j;
        for (j = 0; j < i; j++)
            TLB[j] = TLB[j+1];
    }

    // Insert Entry
    TLB[i].page_number = pageNumber;
    TLB[i].frame_number = frameNumber;

    if (TLB_Full_Entries < TLB_SIZE-1)
        TLB_Full_Entries++;

}


int main(int argc, char const *argv[])
{
    printf("WELCOME TO VIRTUAL MEMORY MANAGER\n");
    printf("Result is outputed to the result.txt file\n");

    if (argc != 2){
        fprintf(stderr, "Usage: ./a.out [input file]\n");
        exit(-1);
    }
    address_file = fopen(argv[1], "r");
    backing_store = fopen("BACKING_STORE.bin", "rb");

    if (address_file == NULL){
        fprintf(stderr, "Error opening addresses.txt %s\n", argv[1]);
        exit(-1);
    }
    int translated_address = 0;
    int logical_address;

    while (fgets(address, ADDRESS_SIZE, address_file) != NULL)
    {
        logical_address = atoi(address);
        // Get the physical address and value stored in that address
        get_page(logical_address);
        translated_address++;
    }
    // printf("Number of translated address = %d\n", translated_address);
    // double pfRate = page_fault / (double) translated_address;
    // double TLBRate = TLBHits / (double) translated_address;

    // printf("Page Fault: %d\n", page_fault);
    // printf("Page Fault Rate: %.3f\n", pfRate);
    // printf("TLB Hits: %.d\n", TLBHits);
    // printf("TLB Hits Rate: %.3f\n", TLBRate);

    fclose(address_file);
    fclose(backing_store);
    
}







